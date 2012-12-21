#include <sasl/include/codegen/cg_vs.h>

#include <sasl/include/codegen/cg_contexts.h>
#include <sasl/include/codegen/cg_module_impl.h>
#include <sasl/include/codegen/cg_impl.imp.h>
#include <sasl/include/codegen/generate_entry.h>

#include <sasl/include/semantic/pety.h>
#include <sasl/include/semantic/semantics.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/expression.h>
#include <sasl/include/syntax_tree/statement.h>
#include <sasl/include/syntax_tree/program.h>
#include <sasl/include/host/utility.h>
#include <sasl/enums/enums_utility.h>
#include <eflib/include/diagnostics/assert.h>
#include <eflib/include/utility/unref_declarator.h>

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

#define SASL_VISITOR_TYPE_NAME cg_vs

using salviar::sv_usage;
using salviar::su_buffer_in;
using salviar::su_buffer_out;
using salviar::su_stream_in;
using salviar::su_stream_out;
using salviar::sv_usage_count;

using salviar::sv_layout;

using sasl::semantic::node_semantic;
using sasl::semantic::symbol;
using sasl::semantic::node_semantic;
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

BEGIN_NS_SASL_CODEGEN();

// expressions
SASL_VISIT_DEF_UNIMPL( cast_expression );
SASL_VISIT_DEF_UNIMPL( expression_list );
SASL_VISIT_DEF_UNIMPL( cond_expression );

SASL_VISIT_DEF( member_expression ){
	EFLIB_UNREF_DECLARATOR(data);

	visit_child(v.expr);
	node_context* agg_ctxt = node_ctxt(v.expr);
	assert(agg_ctxt);
	
	// Aggregated value
	node_semantic* tisi = sem_->get_semantic( v.expr.get() );
	node_context* ctxt = node_ctxt(v, true);

	if( tisi->ty_proto()->is_builtin() ){
		// Swizzle or write mask
		uint32_t masks = sem_->get_semantic(&v)->swizzle();
		multi_value agg_value = agg_ctxt->node_value;
		if( is_scalar(tisi->ty_proto()->tycode) ){
			agg_value = service()->cast_s2v(agg_value);
		}
		ctxt->node_value = service()->emit_extract_elem_mask( agg_value, masks );
	} else {
		// Member
		symbol* struct_sym = sem_->get_symbol( tisi->ty_proto() );
		symbol* mem_sym = struct_sym->find_this( v.member->str );
		assert( mem_sym );

		if( agg_ctxt->is_semantic_mode ){
			node_semantic* param_member_sem = sem_->get_semantic( mem_sym->associated_node() );
			assert( param_member_sem && param_member_sem->ty_proto()->is_builtin() );

			salviar::semantic_value const& sem = param_member_sem->semantic_value_ref();
			sv_layout* psvl = abii->input_sv_layout( sem );
			bool from_cache = layout_to_node_context(ctxt, psvl, false, true);
			// Value and type must get from cache
			assert(from_cache);
		} else {
			node_context* mem_ctxt = node_ctxt( mem_sym->associated_node(), true );
			assert( mem_ctxt );
			ctxt->node_value = mem_ctxt->node_value;
			ctxt->node_value.parent( agg_ctxt->node_value );
			ctxt->node_value.abi( agg_ctxt->node_value.abi() );
		}
	}
}

SASL_VISIT_DEF( variable_expression ){
	// T ODO Referenced symbol must be evaluated in semantic analysis stages.
	symbol* sym = find_symbol(v.var_name->str);
	assert(sym);
	
	// var_si is not null if sym is global value( sv_none is available )
	sv_layout* var_si = abii->input_sv_layout(sym);

	node_context* varctxt = node_ctxt( sym->associated_node() );
	node_context* ctxt = node_ctxt(v, true);
	if( var_si ){
		// TODO: global only available in entry function.
		assert( is_entry( service()->fn().fn ) );
		ctxt->node_value = varctxt->node_value;
		return;
	}

	// Argument("virtual args") or local variable or in non-entry
	cg_impl::visit( v, data );
}

SASL_VISIT_DEF_UNIMPL( identifier );

// declaration & type specifier
SASL_VISIT_DEF_UNIMPL( initializer );
SASL_VISIT_DEF_UNIMPL( member_initializer );
SASL_VISIT_DEF_UNIMPL( declaration );

SASL_VISIT_DEF_UNIMPL( type_definition );
SASL_VISIT_DEF_UNIMPL( tynode );

SASL_VISIT_DEF_UNIMPL( alias_type );

// In cg_vs, you would initialize entry function before call
SASL_SPECIFIC_VISIT_DEF( before_decls_visit, program ){
	// Call parent for initialization
	parent_class::before_decls_visit( v, data );
}

SASL_SPECIFIC_VISIT_DEF( bin_logic, binary_expression ){
	EFLIB_UNREF_DECLARATOR(data);
	multi_value ret_value = emit_logic_op(v.op, v.left_expr, v.right_expr);
	node_ctxt(v, true)->node_value = ret_value.to_rvalue();
}

SASL_SPECIFIC_VISIT_DEF( create_fnsig, function_def ){
	
	if( !entry_fn && abii->is_entry( sem_->get_symbol(&v) ) ){

		node_context* ctxt = node_ctxt(v, true);

		vector<Type*> param_types = generate_vs_entry_param_type( abii, target_data, service() );

		FunctionType* fntype = FunctionType::get( Type::getVoidTy( cg_impl::context() ), param_types, false );
		Function* fn = Function::Create(
			fntype, Function::ExternalLinkage,
			sem_->get_symbol(&v)->mangled_name().raw_string(), cg_impl::module()
			);
		fn->addFnAttr( Attribute::constructStackAlignmentFromInt(16) );
		entry_fn = fn;
		entry_sym = sem_->get_symbol(&v);

		ctxt->function_scope = ctxt_->create_cg_function();
		ctxt->function_scope->fn = fn;
		ctxt->function_scope->fn_def = &v;
		ctxt->function_scope->cg = service();
	} else {
		parent_class::create_fnsig(v, data);
	}
}

SASL_SPECIFIC_VISIT_DEF( create_fnargs, function_def ){
	Function* fn = node_ctxt(v)->function_scope->fn;

	if( abii->is_entry( sem_->get_symbol(&v) ) ){
		// Create entry arguments.
		Function::arg_iterator arg_it = fn->arg_begin();

		arg_it->setName( ".arg.stri" );
		param_values[su_stream_in]  = service()->create_value(
			builtin_types::none, value_array(1, arg_it), value_kinds::reference, abis::c);
		++arg_it;

		arg_it->setName( ".arg.bufi" );
		param_values[su_buffer_in]  = service()->create_value(
			builtin_types::none, value_array(1, arg_it), value_kinds::reference, abis::c);
		++arg_it;

		arg_it->setName( ".arg.stro" );
		param_values[su_stream_out] = service()->create_value(
			builtin_types::none, value_array(1, arg_it), value_kinds::reference, abis::c);
		++arg_it;

		arg_it->setName( ".arg.bufo" );
		param_values[su_buffer_out] = service()->create_value(
			builtin_types::none, value_array(1, arg_it), value_kinds::reference, abis::c);
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
		
		node_semantic* par_ssi	= sem_->get_semantic(param);
		symbol*		   par_sym	= par_ssi->associated_symbol();

		node_context* pctxt = node_ctxt(param, true);

		// Create local variable for 'virtual argument' and 'virtual result'.
		if( par_ssi->ty_proto()->is_builtin() ){
			// Virtual args for built in typed argument.

			// Get Value from semantic.
			// Store value to local variable.
			salviar::semantic_value const& par_sem = par_ssi->semantic_value_ref();
			assert( par_sem != salviar::sv_none);
			sv_layout* psi = abii->input_sv_layout(par_sem);

			builtin_types hint = par_ssi->ty_proto()->tycode;
			pctxt->node_value = service()->create_variable(hint, abis::c, param->name->str);
			layout_to_node_context(
				pctxt, psi, 
				true,						/*store if value existed*/ 
				sem_->is_modified(par_sym)	/*copy from input*/
				);
		} else {
			// Virtual args for aggregated argument
			pctxt->is_semantic_mode = true;
			bool is_param_modified = sem_->is_modified(par_sym);

			// Add values and types of all used semantics to cache.
			if( param_type->node_class() == node_ids::struct_type )
			{
				struct_type* param_struct = static_cast<struct_type*>(param_type);
				BOOST_FOREACH(shared_ptr<declaration> const& decl, param_struct->decls)
				{
					shared_ptr<variable_declaration> var_decl = decl->as_handle<variable_declaration>();
					if(!var_decl) { continue; }
					BOOST_FOREACH(shared_ptr<declarator> const& dclr, var_decl->declarators)
					{
						node_semantic* dclr_sem = sem_->get_semantic(dclr);
						dclr_sem->semantic_value_ref() ;

						salviar::semantic_value const& sem_value = dclr_sem->semantic_value_ref();
						sv_layout* psvl = abii->input_sv_layout(sem_value);
						layout_to_node_context(NULL, psvl, false, is_param_modified);
					}
				}
			}
			else
			{
				EFLIB_ASSERT_UNIMPLEMENTED();
			}
		}
	}
	
	// Get globals address to node context.
	BOOST_FOREACH( symbol* gsym, sem_->global_vars() ){
		node_semantic* pssi = sem_->get_semantic( gsym->associated_node() );

		// Global is filled by offset value with null parent.
		// The parent is filled when it is referred.
		sv_layout* svl = NULL;
		if( pssi->semantic_value_ref() == salviar::sv_none ){
			svl = abii->input_sv_layout( gsym );
		} else {
			svl = abii->input_sv_layout( pssi->semantic_value_ref() );
		}

		layout_to_node_context(
			node_ctxt(gsym->associated_node(), true), svl,
			false, sem_->is_modified(gsym)
			);

		//if (v.init){
		//	EFLIB_ASSERT_UNIMPLEMENTED();
		//}
	}
}

SASL_SPECIFIC_VISIT_DEF( visit_return, jump_statement ){
	EFLIB_UNREF_DECLARATOR(data);

	if( is_entry( service()->fn().fn ) ){
		visit_child( v.jump_expr );

		// Copy result to semantic storage.
		multi_value ret_value = node_ctxt( v.jump_expr )->node_value;

		if( ret_value.hint() != builtin_types::none ){
			// Builtin: Copy directly.
			node_semantic* ret_ssi = sem_->get_semantic(service()->fn().fn_def);
			sv_layout* ret_si = abii->input_sv_layout( ret_ssi->semantic_value_ref() );
			assert( ret_si );
			layout_to_value(ret_si, false).store(ret_value);
		} else {
			// Struct: Copy member-by-member.
			shared_ptr<struct_type> ret_struct = service()->fn().fn_def->type->result_type->as_handle<struct_type>();
			size_t member_index = 0;
			BOOST_FOREACH( shared_ptr<declaration> const& child, ret_struct->decls ){
				if( child->node_class() == node_ids::variable_declaration ){
					shared_ptr<variable_declaration> vardecl = child->as_handle<variable_declaration>();
					BOOST_FOREACH( shared_ptr<declarator> const& decl, vardecl->declarators ){
						node_semantic* decl_ssi = sem_->get_semantic(decl);
						sv_layout* decl_si = abii->output_sv_layout( decl_ssi->semantic_value_ref() );
						assert( decl_si );
						layout_to_value(decl_si, false).store( service()->emit_extract_val(ret_value, (int)member_index) );
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

cg_vs::cg_vs()
	: entry_fn(NULL), entry_sym(NULL)
{
	service_ = new cgs_sisd();
}

bool cg_vs::is_entry( llvm::Function* fn ) const{
	assert(fn && entry_fn);
	return fn && fn == entry_fn;
}

cg_module_impl* cg_vs::mod_ptr(){
	return llvm_mod_.get();
}

multi_value cg_vs::layout_to_value(sv_layout* svl, bool copy_from_input)
{
	if(copy_from_input) EFLIB_ASSERT_UNIMPLEMENTED();

	multi_value ret;

	// TODO: need to emit_extract_ref
	if( svl->usage == su_stream_in || svl->usage == su_stream_out || svl->agg_type == salviar::aggt_array ){
		ret = service()->emit_extract_val(
			param_values[svl->usage],
			static_cast<int>(svl->physical_index)
			);
		ret = ret.as_ref();
	} else {
		ret = service()->emit_extract_ref(
			param_values[svl->usage],
			static_cast<int>(svl->physical_index)
			);
	}

	assert(svl->value_type != salviar::lvt_none);
	ret.hint( to_builtin_types(svl->value_type) );

	return ret;
}

bool cg_vs::layout_to_node_context(
	node_context* psc, salviar::sv_layout* svl,
	bool store_to_existed_value, bool copy_from_input
	)
{
	bool could_cached =
		(svl->usage == su_stream_in || svl->usage == su_buffer_in)
		&& svl->sv.get_system_value() != salviar::sv_none;

	assert(psc || could_cached);

	// Find cached node context prototype.
	if(could_cached)
	{
		input_copies_dict::iterator it = input_copies_.find(svl->sv);

		if( it != input_copies_.end() )
		{
			// Fetch from cache.

			// No output.
			if(!psc) return false;

			// Copy cached value and ty to output context
			if( store_to_existed_value && psc->node_value.storable() ){
				psc->node_value.store( it->second->node_value );
			} else {
				psc->node_value = it->second->node_value;
			}
			psc->ty = it->second->ty;
			return true;
		}
	}

	// if psc is not existed, create a temporary node context.
	if(!psc) psc = ctxt_->create_temporary_node_context();

	if(could_cached){
		input_copies_.insert( make_pair(svl->sv, psc) );
	}

	// Otherwise create value and type for layout, and fill into context.
	builtin_types bt = to_builtin_types(svl->value_type);
	multi_value layout_value;
	// TODO: need to emit_extract_ref
	if( svl->usage == su_stream_in || svl->usage == su_stream_out || svl->agg_type == salviar::aggt_array ){
		layout_value = service()->emit_extract_val(
			param_values[svl->usage],
			static_cast<int>(svl->physical_index)
			);
		layout_value = layout_value.as_ref();
	} else {
		layout_value = service()->emit_extract_ref(
			param_values[svl->usage],
			static_cast<int>(svl->physical_index)
			);
	}

	if(svl->internal_type == -1)
	{
		layout_value.hint( to_builtin_types(svl->value_type) );
	}
	else
	{
		psc->ty = service()->create_ty( sem_->pety()->get_proto(svl->internal_type) );
		layout_value.ty(psc->ty);
	}

	if( store_to_existed_value && psc->node_value.storable() )
	{
		psc->node_value.store(layout_value);
	}
	else
	{
		if(copy_from_input)
		{
			// TODO: only support builtin type copy.
			//		Need to support array copy later.
			assert( layout_value.hint() != builtin_types::none );
			multi_value copied_var = service()->create_variable(layout_value.hint(), layout_value.abi(), ".arg.copy");
			copied_var.store(layout_value);
			layout_value = copied_var;
		}

		psc->node_value = layout_value;
	}

	return false;
}

cg_vs::~cg_vs(){}

END_NS_SASL_CODEGEN();
