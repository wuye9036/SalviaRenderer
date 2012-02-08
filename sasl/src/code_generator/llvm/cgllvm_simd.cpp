#include <sasl/include/code_generator/llvm/cgllvm_simd.h>

#include <sasl/include/host/utility.h>
#include <sasl/include/code_generator/llvm/cgllvm_impl.imp.h>
#include <sasl/include/code_generator/llvm/utility.h>
#include <sasl/include/code_generator/llvm/cgllvm_contexts.h>
#include <sasl/include/semantic/abi_info.h>
#include <sasl/include/semantic/semantic_infos.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/statement.h>
#include <sasl/include/syntax_tree/expression.h>

#include <eflib/include/platform/disable_warnings.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/DerivedTypes.h>
#include <llvm/Target/TargetData.h>
#include <llvm/Function.h>
#include <llvm/Constants.h>
#include <eflib/include/platform/enable_warnings.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/foreach.hpp>
#include <boost/bind.hpp>
#include <eflib/include/platform/boost_end.h>

using sasl::utility::to_builtin_types;
using namespace sasl::syntax_tree;
using namespace sasl::semantic;
using salviar::sv_usage;
using salviar::su_none;
using salviar::su_stream_in;
using salviar::su_stream_out;
using salviar::su_buffer_in;
using salviar::su_buffer_out;
using salviar::storage_usage_count;
using salviar::sv_layout;
using llvm::Type;
using llvm::StructType;
using llvm::StructLayout;
using llvm::PointerType;
using llvm::FunctionType;
using llvm::Function;
using boost::any;
using boost::bind;
using boost::shared_ptr;
using std::vector;

#define SASL_VISITOR_TYPE_NAME cgllvm_simd

BEGIN_NS_SASL_CODE_GENERATOR();

cgllvm_simd::cgllvm_simd(): entry_fn(NULL){
	memset( entry_structs, 0, sizeof( entry_structs ) );
}

cgllvm_simd::~cgllvm_simd(){}

cg_service* cgllvm_simd::service() const{
	return const_cast<cgllvm_simd*>(this);
}

abis cgllvm_simd::local_abi( bool /*is_c_compatible*/ ) const
{
	return abi_package;
}

void cgllvm_simd::create_entries()
{
	create_entry_param( su_stream_in );
	create_entry_param( su_stream_out );
	create_entry_param( su_buffer_in );
	create_entry_param( su_buffer_out );
}

void cgllvm_simd::create_entry_param( sv_usage usage )
{
	vector<sv_layout*> svls = abii->layouts(usage);
	vector<Type*>& tys = entry_tys[usage];

	BOOST_FOREACH( sv_layout* si, svls ){
		builtin_types storage_bt = to_builtin_types(si->value_type);

		entry_tyns[usage].push_back( storage_bt );
		
		if( su_stream_in == usage || su_stream_out == usage ){
			Type* storage_ty = type_( storage_bt, abi_package );
			tys.push_back( storage_ty );
		} else {
			Type* storage_ty = type_( storage_bt, abi_llvm );
			tys.push_back( storage_ty );
		}
	}

	char const* struct_name = NULL;
	switch( usage ){
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
	StructType* out_struct = tys.empty()
		? StructType::create( struct_name, type_(builtin_types::_sint8, abi_llvm), NULL )
		: StructType::create( tys, struct_name );

	entry_structs[usage] = out_struct;

	// Update Layout physical informations.
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
		}
		
		svls[i_elem]->element_padding = (next_offset - offset) - svls[i_elem]->element_size;
	}
}

SASL_VISIT_DEF_UNIMPL( unary_expression );
SASL_VISIT_DEF_UNIMPL( cast_expression );
SASL_VISIT_DEF_UNIMPL( expression_list );
SASL_VISIT_DEF_UNIMPL( cond_expression );
SASL_VISIT_DEF_UNIMPL( index_expression );

SASL_VISIT_DEF( member_expression ){
	any child_ctxt = *data;
	sc_ptr(child_ctxt)->clear_data();
	visit_child( child_ctxt, v.expr );
	cgllvm_sctxt* agg_ctxt = cgllvm_impl::node_ctxt( v.expr );
	assert( agg_ctxt );
	
	// Aggregated value
	type_info_si* tisi = dynamic_cast<type_info_si*>( v.expr->semantic_info().get() );

	if( tisi->type_info()->is_builtin() ){
		// Swizzle or write mask
		uint32_t masks = v.si_ptr<storage_si>()->swizzle();
		value_t agg_value = agg_ctxt->value();
		sc_ptr(data)->value() = emit_extract_elem_mask( agg_value, masks );
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
			cgllvm_sctxt* mem_ctxt = cgllvm_impl::node_ctxt( mem_sym->node() );
			assert( mem_ctxt );
			sc_ptr(data)->value() = mem_ctxt->value();
			sc_ptr(data)->value().parent( agg_ctxt->value() );
			sc_ptr(data)->value().abi( agg_ctxt->value().abi() );
		}
	}

	cgllvm_impl::node_ctxt(v, true)->copy( sc_ptr(data) );
}

SASL_VISIT_DEF( variable_expression ){
	// T ODO Referenced symbol must be evaluated in semantic analysis stages.
	shared_ptr<symbol> sym = find_symbol( sc_ptr(data), v.var_name->str );
	assert(sym);
	
	// var_si is not null if sym is global value( sv_none is available )
	sv_layout* var_si = abii->input_sv_layout( sym );

	cgllvm_sctxt* varctxt = cgllvm_impl::node_ctxt( sym->node() );
	if( var_si ){
		// TODO global only available in entry function.
		assert( fn().fn == entry_fn );
		sc_ptr(data)->value() = varctxt->value();
		cgllvm_impl::node_ctxt(v, true)->copy( sc_ptr(data) );
		return;
	}

	// Argument("virtual args") or local variable or not in entry
	parent_class::visit( v, data );
}

// declaration & type specifier
SASL_VISIT_DEF_UNIMPL( initializer );
SASL_VISIT_DEF_UNIMPL( member_initializer );
SASL_VISIT_DEF_UNIMPL( declaration );
SASL_VISIT_DEF_UNIMPL( type_definition );
SASL_VISIT_DEF_UNIMPL( tynode );
SASL_VISIT_DEF_UNIMPL( array_type );
SASL_VISIT_DEF_UNIMPL( alias_type );

// statement
SASL_VISIT_DEF_UNIMPL( statement );
SASL_VISIT_DEF( if_statement )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
}

SASL_VISIT_DEF_UNIMPL( while_statement );
SASL_VISIT_DEF_UNIMPL( dowhile_statement );
SASL_VISIT_DEF_UNIMPL( for_statement );
SASL_VISIT_DEF_UNIMPL( case_label );
SASL_VISIT_DEF_UNIMPL( ident_label );
SASL_VISIT_DEF_UNIMPL( switch_statement );

SASL_VISIT_DEF( compound_statement ){
	any child_ctxt_init = *data;
	any child_ctxt;

	sc_env_ptr(&child_ctxt_init)->sym = v.symbol();

	for ( std::vector< boost::shared_ptr<statement> >::iterator it = v.stmts.begin();
		it != v.stmts.end(); ++it)
	{
		visit_child( child_ctxt, child_ctxt_init, *it );
	}

	cgllvm_impl::node_ctxt(v, true)->copy( sc_ptr(data) );
}

SASL_VISIT_DEF_UNIMPL( labeled_statement );

SASL_SPECIFIC_VISIT_DEF( before_decls_visit, program )
{
	parent_class::before_decls_visit( v, data );
	create_entries();
}

void add_type_ref( Type* ty, vector<Type*>& tys )
{
	tys.push_back( PointerType::getUnqual( ty ) );
}

SASL_SPECIFIC_VISIT_DEF( create_fnsig, function_type )
{
	if( !entry_fn && abii->is_entry( v.symbol() ) ){

		boost::any child_ctxt;

		vector<Type*> param_types;

		add_type_ref( entry_structs[su_stream_in], param_types );
		add_type_ref( entry_structs[su_buffer_in], param_types );
		add_type_ref( entry_structs[su_stream_out], param_types );
		add_type_ref( entry_structs[su_buffer_out], param_types );

		FunctionType* fntype = FunctionType::get( Type::getVoidTy( cgllvm_impl::context() ), param_types, false );
		Function* fn = Function::Create( fntype, Function::ExternalLinkage, v.symbol()->mangled_name(), cgllvm_impl::module() );
		entry_fn = fn;
		// entry_sym = v.symbol().get();

		sc_data_ptr(data)->self_fn.fn = fn;
		sc_data_ptr(data)->self_fn.fnty = &v;
	} else {
		parent_class::create_fnsig(v, data);
	}
}
SASL_SPECIFIC_VISIT_DEF( create_fnargs, function_type )
{
	Function* fn = sc_data_ptr(data)->self_fn.fn;

	if( abii->is_entry( v.symbol() ) ){
		// Create entry arguments.
		Function::arg_iterator arg_it = fn->arg_begin();

		arg_it->setName( ".arg.stri" );
		entry_values[su_stream_in] = create_value( builtin_types::none, arg_it, vkind_ref, abi_package );
		++arg_it;
		arg_it->setName( ".arg.bufi" );
		entry_values[su_buffer_in] = create_value( builtin_types::none, arg_it, vkind_ref, abi_c );
		++arg_it;
		arg_it->setName( ".arg.stro" );
		entry_values[su_stream_out] = create_value( builtin_types::none, arg_it, vkind_ref, abi_package );
		++arg_it;
		arg_it->setName( ".arg.bufo" );
		entry_values[su_buffer_out] = create_value( builtin_types::none, arg_it, vkind_ref, abi_c );
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

		cgllvm_sctxt* pctxt = cgllvm_impl::node_ctxt( par, true );
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

		cgllvm_impl::node_ctxt( gsym->node(), true )->value() = layout_to_value(psi);

		//if (v.init){
		//	EFLIB_ASSERT_UNIMPLEMENTED();
		//}
	}
}

SASL_SPECIFIC_VISIT_DEF( visit_return	, jump_statement ){
	if( fn().fn == entry_fn ){
		any child_ctxt_init = *data;
		sc_ptr(child_ctxt_init)->clear_data();
		any child_ctxt;

		visit_child( child_ctxt, child_ctxt_init, v.jump_expr );

		// Copy result.
		value_t ret_value = cgllvm_impl::node_ctxt( v.jump_expr )->value();

		if( ret_value.hint() != builtin_types::none ){
			storage_si* ret_ssi = fn().fnty->si_ptr<storage_si>();
			sv_layout* ret_si = abii->output_sv_layout( ret_ssi->get_semantic() );
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
SASL_SPECIFIC_VISIT_DEF( visit_continue	, jump_statement ){
	EFLIB_ASSERT_UNIMPLEMENTED();
}
SASL_SPECIFIC_VISIT_DEF( visit_break	, jump_statement ){
	EFLIB_ASSERT_UNIMPLEMENTED();
}

SASL_SPECIFIC_VISIT_DEF( bin_logic, binary_expression ){
	EFLIB_ASSERT_UNIMPLEMENTED();
}
value_t cgllvm_simd::layout_to_value( sv_layout* svl )
{
	builtin_types bt = to_builtin_types( svl->value_type );
	value_t ret = emit_extract_ref( entry_values[svl->usage], svl->physical_index );
	ret.hint( to_builtin_types( svl->value_type ) );
	return ret;
}

END_NS_SASL_CODE_GENERATOR();