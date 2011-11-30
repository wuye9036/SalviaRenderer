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
#include <llvm/Support/IRBuilder.h>
#include <eflib/include/platform/enable_warnings.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <eflib/include/platform/boost_end.h>

#define SASL_VISITOR_TYPE_NAME cgllvm_vs

using salviar::storage_classifications;
using salviar::sc_buffer_in;
using salviar::sc_buffer_out;
using salviar::sc_stream_in;
using salviar::sc_stream_out;
using salviar::storage_classifications_count;

using salviar::storage_info;

using sasl::semantic::storage_si;
using sasl::semantic::symbol;
using sasl::semantic::type_info_si;

using namespace sasl::syntax_tree;
using namespace llvm;
using namespace sasl::utility;

using boost::any;
using boost::bind;
using boost::shared_ptr;
using std::vector;

#define FUNCTION_SCOPE( fn ) \
	push_fn( (fn) );	\
	scope_guard<void> pop_fn_on_exit##__LINE__( bind( &cg_service::pop_fn, this ) );

BEGIN_NS_SASL_CODE_GENERATOR();

void cgllvm_vs::fill_llvm_type_from_si( storage_classifications st ){
	vector<storage_info*> sis = abii->storage_infos( st );
	BOOST_FOREACH( storage_info* si, sis ){
		builtin_types storage_bt = to_builtin_types(si->value_type);
		entry_param_tys[st].push_back( storage_bt );
		Type* storage_ty = type_( storage_bt, abi_c );

		if( sc_stream_in == st || sc_stream_out == st ){
			entry_params_types[st].push_back( PointerType::getUnqual( storage_ty ) );
		} else {
			entry_params_types[st].push_back( storage_ty );
		}
	}
	
	// Here we create packed data.
	// It is easy to compute struct layout.
	// TODO support aligned and unpacked layout in future.
	entry_params_structs[st].data() = StructType::get( mod_ptr()->context(), entry_params_types[st], true );

	char const* struct_name = NULL;
	switch( st ){
	case sc_stream_in:
		struct_name = ".s.stri";
		break;
	case sc_buffer_in:
		struct_name = ".s.bufi";
		break;
	case sc_stream_out:
		struct_name = ".s.stro";
		break;
	case sc_buffer_out:
		struct_name = ".s.bufo";
		break;
	}
	assert( struct_name );

	// entry_params_structs[st].data()->setName( struct_name );
}

void cgllvm_vs::create_entry_params(){
	fill_llvm_type_from_si ( sc_buffer_in );
	fill_llvm_type_from_si ( sc_buffer_out );
	fill_llvm_type_from_si ( sc_stream_in );
	fill_llvm_type_from_si ( sc_stream_out );
}

void cgllvm_vs::add_entry_param_type( storage_classifications st, vector<Type*>& par_types ){
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
		// value_t vec_value = agg_ctxt->get_value();
		// mem_ctxt->get_value() = create_extract_elem();
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
			storage_info* psi = abii->input_storage( sem );

			sc_ptr(data)->get_value() = si_to_value( psi );
		} else {
			// If it is not semantic mode, use general code
			cgllvm_sctxt* mem_ctxt = node_ctxt( mem_sym->node(), true );
			assert( mem_ctxt );
			sc_ptr(data)->get_value() = mem_ctxt->get_value();
			sc_ptr(data)->get_value().set_parent( agg_ctxt->get_value() );
			sc_ptr(data)->get_value().set_abi( agg_ctxt->get_value().get_abi() );
		}
	}

	node_ctxt(v, true)->copy( sc_ptr(data) );
}

SASL_VISIT_DEF( variable_expression ){
	// T ODO Referenced symbol must be evaluated in semantic analysis stages.
	shared_ptr<symbol> sym = find_symbol( sc_ptr(data), v.var_name->str );
	assert(sym);
	
	// var_si is not null if sym is global value( sv_none is available )
	storage_info* var_si = abii->input_storage( sym );

	cgllvm_sctxt* varctxt = node_ctxt( sym->node() );
	if( var_si ){
		// TODO global only avaliable in entry function.
		assert( is_entry( fn().fn ) );
		sc_ptr(data)->get_value() = varctxt->get_value();
		node_ctxt(v, true)->copy( sc_ptr(data) );
		return;
	}

	// Argument("virtual args") or local variable or in non-entry
	parent_class::visit( v, data );
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
		add_entry_param_type( sc_stream_in, param_types );
		add_entry_param_type( sc_buffer_in, param_types );
		add_entry_param_type( sc_stream_out, param_types );
		add_entry_param_type( sc_buffer_out, param_types );

		FunctionType* fntype = FunctionType::get( Type::getVoidTy(context()), param_types, false );
		Function* fn = Function::Create( fntype, Function::ExternalLinkage, v.symbol()->mangled_name(), module() );
		entry_fn = fn;
		entry_sym = v.symbol().get();

		sc_data_ptr(data)->self_fn.fn = fn;
		sc_data_ptr(data)->self_fn.fnty = &v;
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
		param_values[sc_stream_in] = create_value( builtin_types::none, arg_it, value_t::kind_ref, abi_c );
		++arg_it;

		arg_it->setName( ".arg.bufi" );
		param_values[sc_buffer_in] = create_value( builtin_types::none, arg_it, value_t::kind_ref, abi_c );
		++arg_it;

		arg_it->setName( ".arg.stro" );
		param_values[sc_stream_out] = create_value( builtin_types::none, arg_it, value_t::kind_ref, abi_c );
		++arg_it;

		arg_it->setName( ".arg.bufo" );
		param_values[sc_buffer_out] = create_value( builtin_types::none, arg_it, value_t::kind_ref, abi_c );
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

	FUNCTION_SCOPE( sc_data_ptr(data)->self_fn );

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
			storage_info* psi = abii->input_storage( par_sem );

			builtin_types hint = par_ssi->type_info()->tycode;
			pctxt->get_value() = create_variable( hint, abi_c, par->name->str );
			pctxt->get_value().store( si_to_value(psi) );
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
		storage_info* psi = NULL;
		if( pssi->get_semantic() == salviar::sv_none ){
			psi = abii->input_storage( gsym );
		} else {
			psi = abii->input_storage( pssi->get_semantic() );
		}

		node_ctxt( gsym->node(), true )->get_value() = si_to_value(psi);

		//if (v.init){
		//	EFLIB_ASSERT_UNIMPLEMENTED();
		//}
	}
}

SASL_SPECIFIC_VISIT_DEF( return_statement, jump_statement ){

	if( is_entry( fn().fn ) ){
		any child_ctxt_init = *data;
		sc_ptr(child_ctxt_init)->clear_data();
		any child_ctxt;

		visit_child( child_ctxt, child_ctxt_init, v.jump_expr );

		// Copy result.
		value_t ret_value = node_ctxt( v.jump_expr )->get_value();

		if( ret_value.get_hint() != builtin_types::none ){
			storage_si* ret_ssi = fn().fnty->si_ptr<storage_si>();
			storage_info* ret_si = abii->input_storage( ret_ssi->get_semantic() );
			assert( ret_si );
			si_to_value(ret_si).store( ret_value );
		} else {
			shared_ptr<struct_type> ret_struct = fn().fnty->retval_type->as_handle<struct_type>();
			size_t member_index = 0;
			BOOST_FOREACH( shared_ptr<declaration> const& child, ret_struct->decls ){
				if( child->node_class() == node_ids::variable_declaration ){
					shared_ptr<variable_declaration> vardecl = child->as_handle<variable_declaration>();
					BOOST_FOREACH( shared_ptr<declarator> const& decl, vardecl->declarators ){
						storage_si* decl_ssi = decl->si_ptr<storage_si>();
						storage_info* decl_si = abii->output_storage( decl_ssi->get_semantic() );
						assert( decl_si );
						si_to_value(decl_si).store( emit_extract_val(ret_value, (int)member_index) );
						++member_index;
					}
				}
			}
		}
		
		// Emit entry return.
		emit_return();
	} else {
		parent_class::return_statement(v, data);
	}
}

SASL_SPECIFIC_VISIT_DEF( visit_global_declarator, declarator ){
	sc_env_ptr(data)->sym = v.symbol();
	node_ctxt(v, true)->copy( sc_ptr(data) );
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

bool cgllvm_vs::create_mod( sasl::syntax_tree::program& v )
{
	if ( mod ){ return false; }
	mod = create_codegen_context<cgllvm_modvs>( v.as_handle() );
	return true;
}

boost::shared_ptr<sasl::semantic::symbol> cgllvm_vs::find_symbol( cgllvm_sctxt* data, std::string const& str ){
	return data->env().sym.lock()->find( str );
}

value_t cgllvm_vs::si_to_value( storage_info* si )
{
	builtin_types bt = to_builtin_types( si->value_type );

	// TODO need to emit_extract_ref
	value_t ret;
	if( si->storage == sc_stream_in || si->storage == sc_stream_out ){
		ret = emit_extract_val( param_values[si->storage], si->physical_index );
		ret = ret.as_ref();
	} else {
		ret = emit_extract_ref( param_values[si->storage], si->physical_index );
	}
	ret.set_hint( to_builtin_types( si->value_type ) );

	return ret;
}

END_NS_SASL_CODE_GENERATOR();
