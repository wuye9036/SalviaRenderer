#include <sasl/include/code_generator/llvm/cgllvm_vs.h>
#include <sasl/include/code_generator/llvm/cgllvm_contexts.h>
#include <sasl/include/code_generator/llvm/cgllvm_globalctxt.h>
#include <sasl/include/semantic/semantic_infos.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/expression.h>
#include <sasl/include/syntax_tree/statement.h>
#include <sasl/include/syntax_tree/program.h>
#include <sasl/include/host/utility.h>
#include <sasl/enums/enums_utility.h>
#include <eflib/include/diagnostics/assert.h>

#include <eflib/include/platform/disable_warnings.h>
#include <llvm/DerivedTypes.h>
#include <llvm/Function.h>
#include <llvm/Module.h>
#include <llvm/Target/TargetData.h>
#include <eflib/include/platform/enable_warnings.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <eflib/include/platform/boost_end.h>

#define SASL_VISITOR_TYPE_NAME cgllvm_vs

using salviar::sv_usage;
using salviar::su_buffer_in;
using salviar::su_buffer_out;
using salviar::su_stream_in;
using salviar::su_stream_out;
using salviar::storage_usage_count;

using salviar::sv_layout;

using sasl::semantic::storage_si;
using sasl::semantic::symbol;
using sasl::semantic::type_info_si;
using sasl::semantic::abi_info;

using namespace sasl::syntax_tree;
using namespace llvm;
using namespace sasl::utility;

using boost::any;
using boost::bind;
using boost::shared_ptr;

using std::vector;
using std::sort;
using std::pair;
using std::transform;
using std::make_pair;

#define FUNCTION_SCOPE( fn ) \
	push_fn( (fn) );	\
	scope_guard<void> pop_fn_on_exit##__LINE__( bind( &cgs_sisd::pop_fn, this ) );

BEGIN_NS_SASL_CODE_GENERATOR();

bool layout_type_pairs_cmp( pair<sv_layout*, Type*> const& lhs, pair<sv_layout*, Type*> const& rhs ){
	return lhs.first->element_size < rhs.first->element_size;
}

/// Re-arrange layouts will Sort up struct members by storage size.
/// For e.g.
///   struct { int i; byte b; float f; ushort us; }
/// Will be arranged to
///   struct { byte b; ushort us; int i; float f; };
/// It will minimize the structure size.
void rearrange_layouts( vector<sv_layout*>& sorted_layouts, vector<Type*>& sorted_tys, vector<sv_layout*> const& layouts, vector<Type*> const& tys, TargetData* target )
{
	size_t layouts_count = layouts.size();
	vector<size_t> elems_size;
	elems_size.reserve( layouts_count );

	vector< pair<sv_layout*, Type*> > layout_ty_pairs;
	layout_ty_pairs.reserve( layouts_count );

	vector<sv_layout*>::const_iterator layout_it = layouts.begin();
	BOOST_FOREACH( Type* ty, tys ){
		size_t sz = target->getTypeStoreSize(ty);
		(*layout_it)->element_size = sz;
		layout_ty_pairs.push_back( make_pair(*layout_it, ty) );
		++layout_it;
	}

	sort( layout_ty_pairs.begin(), layout_ty_pairs.end(), layout_type_pairs_cmp );

	sorted_layouts.clear();
	std::transform( layout_ty_pairs.begin(), layout_ty_pairs.end(), back_inserter(sorted_layouts), boost::bind(&pair<sv_layout*, Type*>::first, _1) );

	sorted_tys.clear();
	std::transform( layout_ty_pairs.begin(), layout_ty_pairs.end(), back_inserter(sorted_tys), boost::bind(&pair<sv_layout*, Type*>::second, _1) );
}

void cgllvm_vs::fill_llvm_type_from_si( sv_usage su ){
	vector<sv_layout*> svls = abii->layouts( su );
	vector<Type*>& tys = entry_params_types[su];

	BOOST_FOREACH( sv_layout* si, svls ){
		builtin_types storage_bt = to_builtin_types(si->value_type);
		entry_param_tys[su].push_back( storage_bt );
		Type* storage_ty = type_( storage_bt, abi_c );

		if( su_stream_in == su || su_stream_out == su ){
			tys.push_back( PointerType::getUnqual( storage_ty ) );
		} else {
			tys.push_back( storage_ty );
		}
	}
	
	if( su_buffer_in == su || su_buffer_out == su ){
		rearrange_layouts( svls, tys, svls, tys, target_data );
	}

	char const* struct_name = NULL;
	switch( su ){
	case su_stream_in:
		struct_name = ".s.stri";
		break;
	case su_buffer_in:
		struct_name = ".s.bufi";
		break;
	case su_stream_out:
		struct_name = ".s.stro";
		break;
	case su_buffer_out:
		struct_name = ".s.bufo";
		break;
	}
	assert( struct_name );

	// Tys must not be empty. So placeholder (int8) will be inserted if tys is empty.
	StructType* out_struct = tys.empty() ? StructType::create( struct_name, type_(builtin_types::_sint8, abi_llvm), NULL ) : StructType::create( tys, struct_name );
	entry_params_structs[su].data() = out_struct;

	// Update Layout physical informations.
	if( su_buffer_in == su || su_buffer_out == su ){
		StructLayout const* struct_layout = target_data->getStructLayout( out_struct );

		size_t next_offset = 0;
		for( size_t i_elem = 0; i_elem < svls.size(); ++i_elem ){
			size_t offset = next_offset;
			svls[i_elem]->offset = offset;
			svls[i_elem]->physical_index = i_elem;

			size_t next_i_elem = i_elem + 1;
			if( next_i_elem < tys.size() ){
				next_offset = struct_layout->getElementOffset( static_cast<unsigned>(next_i_elem) );
			} else {
				next_offset = struct_layout->getSizeInBytes();
				const_cast<abi_info*>(abii)->update_size( next_offset, su );
			}
		
			svls[i_elem]->element_padding = (next_offset - offset) - svls[i_elem]->element_size;
		}
	}
}

void cgllvm_vs::create_entry_params(){
	fill_llvm_type_from_si ( su_buffer_in );
	fill_llvm_type_from_si ( su_buffer_out );
	fill_llvm_type_from_si ( su_stream_in );
	fill_llvm_type_from_si ( su_stream_out );
}

void cgllvm_vs::add_entry_param_type( sv_usage st, vector<Type*>& par_types ){
	StructType* par_type = entry_params_structs[st].data();
	PointerType* parref_type = PointerType::getUnqual( par_type );

	par_types.push_back(parref_type);
}

// expressions
SASL_VISIT_DEF_UNIMPL( unary_expression );
SASL_VISIT_DEF_UNIMPL( cast_expression );
SASL_VISIT_DEF_UNIMPL( expression_list );
SASL_VISIT_DEF_UNIMPL( cond_expression );
SASL_VISIT_DEF_UNIMPL( index_expression );

SASL_VISIT_DEF( member_expression ){
	any child_ctxt = *data;
	sc_ptr(child_ctxt)->clear_data();
	visit_child( child_ctxt, v.expr );
	cgllvm_sctxt* agg_ctxt = node_ctxt( v.expr );
	assert( agg_ctxt );
	
	// Aggregated value
	type_info_si* tisi = dynamic_cast<type_info_si*>( v.expr->semantic_info().get() );

	if( tisi->type_info()->is_builtin() ){
		// Swizzle or write mask
		// storage_si* mem_ssi = v.si_ptr<storage_si>();
		// value_t vec_value = agg_ctxt->value();
		// mem_ctxt->value() = create_extract_elem();
		EFLIB_ASSERT_UNIMPLEMENTED();
	} else {
		// Member
		shared_ptr<symbol> struct_sym = tisi->type_info()->symbol();
		shared_ptr<symbol> mem_sym = struct_sym->find_this( v.member->str );
		assert( mem_sym );

		if( agg_ctxt->data().semantic_mode ){
			storage_si* par_mem_ssi = mem_sym->node()->si_ptr<storage_si>();
			assert( par_mem_ssi && par_mem_ssi->type_info()->is_builtin() );

			salviar::semantic_value const& sem = par_mem_ssi->get_semantic();
			sv_layout* psi = abii->input_sv_layout( sem );

			sc_ptr(data)->value() = layout_to_value( psi );
		} else {
			// If it is not semantic mode, use general code
			cgllvm_sctxt* mem_ctxt = node_ctxt( mem_sym->node(), true );
			assert( mem_ctxt );
			sc_ptr(data)->value() = mem_ctxt->value();
			sc_ptr(data)->value().parent( agg_ctxt->value() );
			sc_ptr(data)->value().abi( agg_ctxt->value().abi() );
		}
	}

	node_ctxt(v, true)->copy( sc_ptr(data) );
}

SASL_VISIT_DEF( variable_expression ){
	// T ODO Referenced symbol must be evaluated in semantic analysis stages.
	shared_ptr<symbol> sym = find_symbol( sc_ptr(data), v.var_name->str );
	assert(sym);
	
	// var_si is not null if sym is global value( sv_none is available )
	sv_layout* var_si = abii->input_sv_layout( sym );

	cgllvm_sctxt* varctxt = node_ctxt( sym->node() );
	if( var_si ){
		// TODO global only avaliable in entry function.
		assert( is_entry( fn().fn ) );
		sc_ptr(data)->value() = varctxt->value();
		node_ctxt(v, true)->copy( sc_ptr(data) );
		return;
	}

	// Argument("virtual args") or local variable or in non-entry
	cgllvm_impl::visit( v, data );
}

SASL_VISIT_DEF_UNIMPL( identifier );

// declaration & type specifier
SASL_VISIT_DEF_UNIMPL( initializer );
SASL_VISIT_DEF_UNIMPL( member_initializer );
SASL_VISIT_DEF_UNIMPL( declaration );

SASL_VISIT_DEF_UNIMPL( type_definition );
SASL_VISIT_DEF_UNIMPL( tynode );
SASL_VISIT_DEF_UNIMPL( array_type );

SASL_VISIT_DEF_UNIMPL( alias_type );

// In cgllvm_vs, you would initialize entry function before call
SASL_SPECIFIC_VISIT_DEF( before_decls_visit, program ){
	// Call parent for initialization
	parent_class::before_decls_visit( v, data );
	// Create entry function
	create_entry_params();
}

SASL_SPECIFIC_VISIT_DEF( create_fnsig, function_type ){
	
	if( !entry_fn && abii->is_entry( v.symbol() ) ){

		boost::any child_ctxt;

		vector<Type*> param_types;
		add_entry_param_type( su_stream_in, param_types );
		add_entry_param_type( su_buffer_in, param_types );
		add_entry_param_type( su_stream_out, param_types );
		add_entry_param_type( su_buffer_out, param_types );

		FunctionType* fntype = FunctionType::get( Type::getVoidTy( cgllvm_impl::context() ), param_types, false );
		Function* fn = Function::Create( fntype, Function::ExternalLinkage, v.symbol()->mangled_name(), cgllvm_impl::module() );
		fn->addFnAttr( Attribute::constructStackAlignmentFromInt(16) );
		entry_fn = fn;
		entry_sym = v.symbol().get();

		sc_data_ptr(data)->self_fn.fn = fn;
		sc_data_ptr(data)->self_fn.fnty = &v;
		sc_data_ptr(data)->self_fn.cg = service();
	} else {
		parent_class::create_fnsig(v, data);
	}
}

SASL_SPECIFIC_VISIT_DEF( create_fnargs, function_type ){
	Function* fn = sc_data_ptr(data)->self_fn.fn;

	if( abii->is_entry( v.symbol() ) ){
		// Create entry arguments.
		Function::arg_iterator arg_it = fn->arg_begin();

		arg_it->setName( ".arg.stri" );
		param_values[su_stream_in] = create_value( builtin_types::none, arg_it, vkind_ref, abi_c );
		++arg_it;

		arg_it->setName( ".arg.bufi" );
		param_values[su_buffer_in] = create_value( builtin_types::none, arg_it, vkind_ref, abi_c );
		++arg_it;

		arg_it->setName( ".arg.stro" );
		param_values[su_stream_out] = create_value( builtin_types::none, arg_it, vkind_ref, abi_c );
		++arg_it;

		arg_it->setName( ".arg.bufo" );
		param_values[su_buffer_out] = create_value( builtin_types::none, arg_it, vkind_ref, abi_c );
		++arg_it;

		// Create virutal arguments
		create_virtual_args(v, data);

	} else {
		parent_class::create_fnargs(v, data);
	}
}

SASL_SPECIFIC_VISIT_DEF( create_virtual_args, function_type ){
	any child_ctxt_init = *data;
	sc_ptr(child_ctxt_init)->clear_data();

	any child_ctxt;

	new_block( ".init.vargs", true );

	BOOST_FOREACH( shared_ptr<parameter> const& par, v.params ){
		visit_child( child_ctxt, child_ctxt_init, par->param_type );
		storage_si* par_ssi = dynamic_cast<storage_si*>( par->semantic_info().get() );

		cgllvm_sctxt* pctxt = node_ctxt( par, true );
		// Create local variable for 'virtual argument' and 'virtual result'.
		pctxt->env( sc_ptr(data) );

		if( par_ssi->type_info()->is_builtin() ){
			// Virtual args for built in typed argument.

			// Get Value from semantic.
			// Store value to local variable.
			salviar::semantic_value const& par_sem = par_ssi->get_semantic();
			assert( par_sem != salviar::sv_none );
			sv_layout* psi = abii->input_sv_layout( par_sem );

			builtin_types hint = par_ssi->type_info()->tycode;
			pctxt->value() = create_variable( hint, abi_c, par->name->str );
			pctxt->value().store( layout_to_value(psi) );
		} else {
			// Virtual args for aggregated argument
			pctxt->data().semantic_mode = true;
		}
	}
	
	// Update globals
	BOOST_FOREACH( shared_ptr<symbol> const& gsym, msi->globals() ){
		storage_si* pssi = gsym->node()->si_ptr<storage_si>();

		// Global is filled by offset value with null parent.
		// The parent is filled when it is referred.
		sv_layout* psi = NULL;
		if( pssi->get_semantic() == salviar::sv_none ){
			psi = abii->input_sv_layout( gsym );
		} else {
			psi = abii->input_sv_layout( pssi->get_semantic() );
		}

		node_ctxt( gsym->node(), true )->value() = layout_to_value(psi);

		//if (v.init){
		//	EFLIB_ASSERT_UNIMPLEMENTED();
		//}
	}
}

SASL_SPECIFIC_VISIT_DEF( visit_return, jump_statement ){
	if( is_entry( fn().fn ) ){
		any child_ctxt_init = *data;
		sc_ptr(child_ctxt_init)->clear_data();
		any child_ctxt;

		visit_child( child_ctxt, child_ctxt_init, v.jump_expr );

		// Copy result.
		value_t ret_value = node_ctxt( v.jump_expr )->value();

		if( ret_value.hint() != builtin_types::none ){
			storage_si* ret_ssi = fn().fnty->si_ptr<storage_si>();
			sv_layout* ret_si = abii->input_sv_layout( ret_ssi->get_semantic() );
			assert( ret_si );
			layout_to_value(ret_si).store( ret_value );
		} else {
			shared_ptr<struct_type> ret_struct = fn().fnty->retval_type->as_handle<struct_type>();
			size_t member_index = 0;
			BOOST_FOREACH( shared_ptr<declaration> const& child, ret_struct->decls ){
				if( child->node_class() == node_ids::variable_declaration ){
					shared_ptr<variable_declaration> vardecl = child->as_handle<variable_declaration>();
					BOOST_FOREACH( shared_ptr<declarator> const& decl, vardecl->declarators ){
						storage_si* decl_ssi = decl->si_ptr<storage_si>();
						sv_layout* decl_si = abii->output_sv_layout( decl_ssi->get_semantic() );
						assert( decl_si );
						layout_to_value(decl_si).store( emit_extract_val(ret_value, (int)member_index) );
						++member_index;
					}
				}
			}
		}
		
		// Emit entry return.
		emit_return();
	} else {
		parent_class::visit_return(v, data);
	}
}

cgllvm_vs::cgllvm_vs(): entry_fn(NULL), entry_sym(NULL){}

bool cgllvm_vs::is_entry( llvm::Function* fn ) const{
	assert(fn && entry_fn);
	return fn && fn == entry_fn;
}

cgllvm_modvs* cgllvm_vs::mod_ptr(){
	assert( dynamic_cast<cgllvm_modvs*>( mod.get() ) );
	return static_cast<cgllvm_modvs*>( mod.get() );
}

value_t cgllvm_vs::layout_to_value( sv_layout* svl )
{
	builtin_types bt = to_builtin_types( svl->value_type );

	// TODO need to emit_extract_ref
	value_t ret;
	if( svl->usage == su_stream_in || svl->usage == su_stream_out ){
		ret = emit_extract_val( param_values[svl->usage], svl->physical_index );
		ret = ret.as_ref();
	} else {
		ret = emit_extract_ref( param_values[svl->usage], svl->physical_index );
	}
	ret.hint( to_builtin_types( svl->value_type ) );

	return ret;
}

cgllvm_vs::~cgllvm_vs(){}

END_NS_SASL_CODE_GENERATOR();
