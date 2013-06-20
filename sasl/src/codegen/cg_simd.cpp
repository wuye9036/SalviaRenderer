#include <sasl/include/codegen/cg_simd.h>

#include <sasl/include/host/utility.h>
#include <sasl/include/codegen/cg_impl.imp.h>
#include <sasl/include/codegen/utility.h>
#include <sasl/include/codegen/cg_contexts.h>
#include <sasl/include/codegen/generate_entry.h>
#include <sasl/include/semantic/reflection_impl.h>
#include <sasl/include/semantic/semantics.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/semantic/caster.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/statement.h>
#include <sasl/include/syntax_tree/expression.h>
#include <eflib/include/utility/unref_declarator.h>
#include <eflib/include/utility/scoped_value.h>

#include <eflib/include/platform/disable_warnings.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
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
using salviar::sv_usage_count;
using salviar::sv_layout;
using eflib::scoped_value;
using llvm::Type;
using llvm::StructType;
using llvm::StructLayout;
using llvm::PointerType;
using llvm::FunctionType;
using llvm::Function;
using boost::bind;
using boost::shared_ptr;
using std::vector;

#define SASL_VISITOR_TYPE_NAME cg_simd

BEGIN_NS_SASL_CODEGEN();

cg_simd::cg_simd(): entry_fn(NULL)
{
	service_ = new cgs_simd();
}

cg_simd::~cg_simd(){}

cgs_simd* cg_simd::service() const{
	return static_cast<cgs_simd*>(service_);
}

SASL_VISIT_DEF_UNIMPL( unary_expression );
SASL_VISIT_DEF_UNIMPL( cast_expression );
SASL_VISIT_DEF_UNIMPL( expression_list );
SASL_VISIT_DEF_UNIMPL( cond_expression );
SASL_VISIT_DEF_UNIMPL( index_expression );

SASL_VISIT_DEF( member_expression ){
	EFLIB_UNREF_DECLARATOR(data);

	visit_child(v.expr);
	node_context* agg_ctxt = node_ctxt( v.expr );
	assert( agg_ctxt );
	
	// Aggregated value
	node_semantic* tisi = sem_->get_semantic(v.expr);
	node_context* ctxt = node_ctxt(v, true);

	if( tisi->ty_proto()->is_builtin() ){
		// Swizzle or write mask
		uint32_t masks = sem_->get_semantic(&v)->swizzle();
		multi_value agg_value = agg_ctxt->node_value;
		ctxt->node_value = service()->emit_extract_elem_mask( agg_value, masks );
	} else {
		// Member
		symbol* struct_sym = sem_->get_symbol( tisi->ty_proto() );
		symbol* mem_sym = struct_sym->find_this( v.member->str );
		assert( mem_sym );

		if( agg_ctxt->is_semantic_mode ){
			node_semantic* par_mem_ssi = sem_->get_semantic( mem_sym->associated_node() );
			assert( par_mem_ssi && par_mem_ssi->ty_proto()->is_builtin() );

			salviar::semantic_value const& sem = par_mem_ssi->semantic_value_ref();
			sv_layout* psi = abii->input_sv_layout( sem );

			ctxt->node_value = layout_to_value( psi );
		} else {
			// If it is not semantic mode, use general code
			node_context* mem_ctxt = cg_impl::node_ctxt( mem_sym->associated_node() );
			assert( mem_ctxt );
			ctxt->node_value = mem_ctxt->node_value;
			ctxt->node_value.parent( agg_ctxt->node_value );
			ctxt->node_value.abi( agg_ctxt->node_value.abi() );
		}
	}
}

SASL_VISIT_DEF( variable_expression ){
	EFLIB_UNREF_DECLARATOR(data);

	// TODO: Referenced symbol must be evaluated in semantic analysis stages.
	symbol* sym = find_symbol(v.var_name->str);
	assert(sym);
	
	// var_si is not null if sym is global value( sv_none is available )
	sv_layout* var_si = abii->input_sv_layout(sym);

	node_context* varctxt = node_ctxt( sym->associated_node() );
	node_context* ctxt = node_ctxt(v, true);

	if( var_si ){
		// TODO: global only available in entry function.
		assert( service()->fn().fn == entry_fn );
		ctxt->node_value = varctxt->node_value;
		return;
	}

	// Argument("virtual args") or local variable or not in entry
	parent_class::visit(v, NULL);
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
	EFLIB_UNREF_DECLARATOR(data);

	service()->if_beg();

	service()->if_cond_beg();
	visit_child( v.cond );
	tid_t cond_tid = sem_->get_semantic(v.cond)->tid();
	tid_t bool_tid = sem_->pety()->get( builtin_types::_boolean );
	if( cond_tid != bool_tid ){
		if( caster->cast( sem_->pety()->get_proto(bool_tid), v.cond.get() ) == caster_t::nocast ){
			assert(false);
		}
	}
	service()->if_cond_end( cg_impl::node_ctxt(v.cond)->node_value );
	insert_point_t ip_cond = service()->insert_point();

	insert_point_t ip_yes_beg = service()->new_block( "if.yes", true );
	service()->then_beg();
	visit_child( v.yes_stmt );
	service()->then_end();
	insert_point_t ip_yes_end = service()->insert_point();

	insert_point_t ip_no_beg, ip_no_end;
	if( v.no_stmt ){
		ip_no_beg = service()->new_block( "if.no", true );
		service()->else_beg();
		visit_child( v.no_stmt );
		service()->else_end();
		ip_no_end = service()->insert_point();
	}

	insert_point_t ip_merge = service()->new_block( "if.merged", false );

	service()->if_end();

	// Link Blocks
	service()->set_insert_point( ip_cond );
	service()->jump_to( ip_yes_beg );

	if( ip_no_beg ){
		service()->set_insert_point( ip_yes_end );
		service()->jump_to( ip_no_beg );

		service()->set_insert_point( ip_no_end );
		service()->jump_to( ip_merge );
	}
	
	service()->set_insert_point( ip_merge );
}

SASL_VISIT_DEF( while_statement )
{
	EFLIB_UNREF_DECLARATOR(data);

	service()->while_beg();
	
	insert_point_t cond_beg = service()->new_block( "while.cond", true );
	service()->while_cond_beg();
	visit_child( v.cond );
	tid_t cond_tid = sem_->get_semantic(v.cond)->tid();
	tid_t bool_tid = sem_->pety()->get( builtin_types::_boolean );
	if( cond_tid != bool_tid ){
		caster->cast(sem_->pety()->get_proto(bool_tid), v.cond.get());
	}
	service()->while_cond_end( cg_impl::node_ctxt(v.cond)->node_value );
	multi_value any_mask_true = service()->any_mask_true();
	insert_point_t cond_end = service()->insert_point();

	insert_point_t body_beg = service()->new_block( "while.body", true );
	service()->while_body_beg();
	visit_child( v.body );
	service()->while_body_end();
	insert_point_t body_end = service()->insert_point();

	insert_point_t while_end = service()->new_block( "while.end", true );
	
	service()->while_end();

	// Fill back
	service()->set_insert_point( cond_end );
	service()->jump_cond( any_mask_true, body_beg, while_end );

	service()->set_insert_point( body_end );
	service()->jump_to( cond_beg );

	service()->set_insert_point( while_end );
}
SASL_VISIT_DEF( dowhile_statement )
{
	EFLIB_UNREF_DECLARATOR(data);

	service()->do_beg();

	insert_point_t body_beg = service()->new_block( "dowhile.body", true );
	service()->do_body_beg();
	visit_child( v.body );
	service()->do_body_end();
	insert_point_t body_end = service()->insert_point();

	insert_point_t cond_beg = service()->new_block( "dowhile.cond", true );
	service()->do_cond_beg();
	visit_child( v.cond );
	tid_t cond_tid = sem_->get_semantic(v.cond)->tid();
	tid_t bool_tid = sem_->pety()->get( builtin_types::_boolean );
	if( cond_tid != bool_tid ){
		if (caster->cast(sem_->pety()->get_proto(bool_tid), v.cond.get()) == caster_t::nocast ){
			assert( false );
		}
	}
	service()->do_cond_end( cg_impl::node_ctxt(v.cond)->node_value );
	multi_value any_mask_true = service()->any_mask_true();
	insert_point_t cond_end = service()->insert_point();

	insert_point_t while_end = service()->new_block( "dowhile.end", true );
	service()->do_end();

	// Fill back
	service()->set_insert_point( body_end );
	service()->jump_to( cond_beg );

	service()->set_insert_point( cond_end );
	service()->jump_cond( any_mask_true, body_beg, while_end );

	service()->set_insert_point( while_end );
}

SASL_VISIT_DEF( for_statement )
{
	EFLIB_UNREF_DECLARATOR(data);
	SYMBOL_SCOPE( sem_->get_symbol(&v) );

	// Pseudo: SIMD For
	//
	//   for initializer
	// for_cond:
	//   cond_mask = cond
	//   current_mask &= cond_mask
	//   if sum( current mask ) == 0
	//     goto for_end
	//   else
	//     goto for_body
	// for_body:
	//   ...
	//   goto for_iter
	// for_iter:
	//   iteration code
	//   goto for_cond
	// for_end:
	//   ...
	
	service()->for_init_beg();
	visit_child( v.init );
	service()->for_init_end();
	insert_point_t init_end = service()->insert_point();

	insert_point_t cond_beg = service()->new_block( "for_cond", true );
	service()->for_cond_beg();
	multi_value cond_value;
	if( v.cond ){ 
		visit_child( v.cond );
		cond_value = cg_impl::node_ctxt( v.cond, false )->node_value;
	}
	service()->for_cond_end( cond_value );
	multi_value any_mask_true = service()->any_mask_true();
	insert_point_t cond_end = service()->insert_point();

	insert_point_t body_beg = service()->new_block( "for_body", true );
	service()->for_body_beg();
	visit_child( v.body );
	service()->for_body_end();
	insert_point_t body_end = service()->insert_point();

	insert_point_t iter_beg = service()->new_block( "for_iter", true );
	service()->for_iter_beg();
	if( v.iter ){ visit_child( v.iter ); }
	service()->for_iter_end();
	insert_point_t iter_end = service()->insert_point();

	insert_point_t for_end = service()->new_block( "for_end", true );

	// Fill back jumps
	service()->set_insert_point( init_end );
	service()->jump_to( cond_beg );

	service()->set_insert_point( cond_end );
	service()->jump_cond( any_mask_true, body_beg, for_end );

	service()->set_insert_point( body_end );
	service()->jump_to( iter_beg );

	service()->set_insert_point( iter_end );
	service()->jump_to( cond_beg );

	// Set correct out block.
	service()->set_insert_point( for_end );
}

SASL_VISIT_DEF_UNIMPL( case_label );
SASL_VISIT_DEF_UNIMPL( ident_label );
SASL_VISIT_DEF_UNIMPL( switch_statement );

SASL_VISIT_DEF( compound_statement ){
	EFLIB_UNREF_DECLARATOR(data);

	SYMBOL_SCOPE( sem_->get_symbol(&v) );
	
	for ( std::vector< boost::shared_ptr<statement> >::iterator it = v.stmts.begin();
		it != v.stmts.end(); ++it)
	{
		visit_child( *it );
	}
}

SASL_VISIT_DEF_UNIMPL( labeled_statement );

SASL_SPECIFIC_VISIT_DEF( before_decls_visit, program )
{
	parent_class::before_decls_visit( v, data );
}

void add_type_ref( Type* ty, vector<Type*>& tys )
{
	tys.push_back( PointerType::getUnqual( ty ) );
}

SASL_SPECIFIC_VISIT_DEF( create_fnsig, function_def )
{
	if( !entry_fn && abii->is_entry( sem_->get_symbol(&v) ) )
	{
		vector<Type*> param_types = generate_ps_entry_param_type( abii, vm_data_layout_, service() );
		FunctionType* fntype = FunctionType::get( Type::getVoidTy( cg_impl::context() ), param_types, false );
		Function* fn = Function::Create(
			fntype, Function::ExternalLinkage,
			sem_->get_symbol(&v)->mangled_name().raw_string(),
			cg_impl::module()
			);
		entry_fn = fn;
		// entry_sym = v.symbol().get();

		node_context* ctxt = node_ctxt(v, true);
		ctxt->function_scope = ctxt_->create_cg_function();
		ctxt->function_scope->fn = fn;
		ctxt->function_scope->fn_def = &v;
		ctxt->function_scope->cg = service();
	} else {
		parent_class::create_fnsig(v, data);
	}
}
SASL_SPECIFIC_VISIT_DEF( create_fnargs, function_def )
{
	node_context* ctxt = node_ctxt(v);
	Function* fn = ctxt->function_scope->fn;

	if( abii->is_entry( sem_->get_symbol(&v) ) ){
		// Create entry arguments.
		Function::arg_iterator arg_it = fn->arg_begin();

		arg_it->setName( ".arg.stri" );
		entry_values[su_stream_in]  = service()->create_value(
			builtin_types::none, service()->extension()->split_array_ref(arg_it),
			value_kinds::reference, abis::c);
		++arg_it;
		arg_it->setName( ".arg.bufi" );
		entry_values[su_buffer_in]  = service()->create_value(
			builtin_types::none, value_array(service()->parallel_factor(), arg_it),
			value_kinds::reference, abis::c);
		++arg_it;
		arg_it->setName( ".arg.stro" );
		entry_values[su_stream_out] = service()->create_value(
			builtin_types::none, service()->extension()->split_array_ref(arg_it),
			value_kinds::reference, abis::c);
		++arg_it;
		arg_it->setName( ".arg.bufo" );
		entry_values[su_buffer_out] = service()->create_value(
			builtin_types::none, value_array(1, arg_it),
			value_kinds::reference, abis::c);
		++arg_it;

		// Create virtual arguments
		create_virtual_args(v, data);
	} else {
		parent_class::create_fnargs(v, data);
	}
}
SASL_SPECIFIC_VISIT_DEF( create_virtual_args, function_def ){
	EFLIB_UNREF_DECLARATOR(data);

	service()->new_block( ".init.vargs", true );
	for(size_t i_param = 0; i_param < v.params.size(); ++i_param)
	{
		parameter*	param = v.params[i_param].get();
		tynode*		param_type = v.type->param_types[i_param].get();

		visit_child(param_type);
		node_semantic* par_ssi = sem_->get_semantic(param);

		node_context* pctxt = node_ctxt(param, true);

		// Create local variable for 'virtual argument' and 'virtual result'.
		if( par_ssi->ty_proto()->is_builtin() )
		{
			// Virtual args for built in typed argument.

			// Get Value from semantic.
			// Store value to local variable.
			salviar::semantic_value const& par_sem = par_ssi->semantic_value_ref();
			assert( par_sem != salviar::sv_none );
			sv_layout* psi = abii->input_sv_layout(par_sem);

			builtin_types hint = par_ssi->ty_proto()->tycode;
			pctxt->node_value = service()->create_variable( hint, service()->param_abi(false), param->name->str );
			pctxt->node_value.store( layout_to_value(psi) );
		}
		else
		{
			// Virtual args for aggregated argument
			pctxt->is_semantic_mode = true;
		}
	}
	
	// Update globals
	BOOST_FOREACH( symbol* gsym, sem_->global_vars() ){
		node_semantic* pssi = sem_->get_semantic( gsym->associated_node() );

		// Global is filled by offset value with null parent.
		// The parent is filled when it is referred.
		sv_layout* psi = NULL;
		if( pssi->semantic_value_ref() == salviar::sv_none ){
			psi = abii->input_sv_layout( gsym );
		} else {
			psi = abii->input_sv_layout( pssi->semantic_value_ref() );
		}

		node_ctxt( gsym->associated_node(), true )->node_value = layout_to_value(psi);

		//if (v.init){
		//	EFLIB_ASSERT_UNIMPLEMENTED();
		//}
	}
}

SASL_SPECIFIC_VISIT_DEF( visit_return	, jump_statement ){
	EFLIB_UNREF_DECLARATOR(data);

	if( service()->fn().fn == entry_fn ){
		visit_child( v.jump_expr );

		// Copy result.
		multi_value ret_value = cg_impl::node_ctxt( v.jump_expr )->node_value;

		if( ret_value.hint() != builtin_types::none ){
			node_semantic* ret_ssi = sem_->get_semantic(service()->fn().fn_def);
			sv_layout* ret_si = abii->output_sv_layout( ret_ssi->semantic_value_ref() );
			assert( ret_si );
			layout_to_value(ret_si).store( ret_value );
		} else {
			shared_ptr<struct_type> ret_struct = service()->fn().fn_def->type->result_type->as_handle<struct_type>();
			size_t member_index = 0;
			BOOST_FOREACH( shared_ptr<declaration> const& child, ret_struct->decls ){
				if( child->node_class() == node_ids::variable_declaration ){
					shared_ptr<variable_declaration> vardecl = child->as_handle<variable_declaration>();
					BOOST_FOREACH( shared_ptr<declarator> const& decl, vardecl->declarators ){
						node_semantic* decl_ssi = sem_->get_semantic(decl);
						sv_layout* decl_si = abii->output_sv_layout( decl_ssi->semantic_value_ref() );
						assert( decl_si );
						layout_to_value(decl_si).store( service()->emit_extract_val(ret_value, (int)member_index) );
						++member_index;
					}
				}
			}
		}
		
		// Emit entry return.
		service()->emit_return();
	} else {
		parent_class::visit_return(v, data);
	}
}
SASL_SPECIFIC_VISIT_DEF( visit_continue	, jump_statement ){
	EFLIB_UNREF_DECLARATOR(data);
	EFLIB_UNREF_DECLARATOR(v);
	EFLIB_ASSERT_UNIMPLEMENTED();
}
SASL_SPECIFIC_VISIT_DEF( visit_break	, jump_statement ){
	EFLIB_UNREF_DECLARATOR(data);
	EFLIB_UNREF_DECLARATOR(v);
	service()->break_();
}

SASL_SPECIFIC_VISIT_DEF( bin_logic, binary_expression ){
	EFLIB_UNREF_DECLARATOR(data);
	EFLIB_UNREF_DECLARATOR(v);
	EFLIB_ASSERT_UNIMPLEMENTED();
}
multi_value cg_simd::layout_to_value( sv_layout* svl )
{
	builtin_types bt = to_builtin_types( svl->value_type );
	multi_value ret = service()->emit_extract_ref(
		entry_values[svl->usage], 
		static_cast<int>(svl->physical_index)
		);
	ret.hint( to_builtin_types( svl->value_type ) );
	return ret;
}

abis::id cg_simd::local_abi( bool /*is_c_compatible*/ ) const
{
	return abis::llvm;
}

END_NS_SASL_CODEGEN();
