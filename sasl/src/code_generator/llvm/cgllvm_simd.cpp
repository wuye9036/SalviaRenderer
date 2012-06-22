#include <sasl/include/code_generator/llvm/cgllvm_simd.h>

#include <sasl/include/host/utility.h>
#include <sasl/include/code_generator/llvm/cgllvm_impl.imp.h>
#include <sasl/include/code_generator/llvm/utility.h>
#include <sasl/include/code_generator/llvm/cgllvm_contexts.h>
#include <sasl/include/semantic/abi_info.h>
#include <sasl/include/semantic/semantics.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/semantic/caster.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/statement.h>
#include <sasl/include/syntax_tree/expression.h>
#include <sasl/include/common/scope_guard.h>

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
using sasl::common::scope_guard;

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

cgs_simd* cgllvm_simd::service() const{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return NULL;
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
			Type* storage_ty = service()->type_( storage_bt, abi_package );
			tys.push_back( storage_ty );
		} else {
			Type* storage_ty = service()->type_( storage_bt, abi_c );
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
		? StructType::create( struct_name, service()->type_(builtin_types::_sint8, abi_llvm), NULL )
		: StructType::create( tys, struct_name, true );

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
		
		svls[i_elem]->element_padding = ( (next_offset - offset) / svls[i_elem]->element_count ) - svls[i_elem]->element_size ;
		svls[i_elem]->padding =			(next_offset - offset) - ( svls[i_elem]->element_size + svls[i_elem]->element_padding ) * svls[i_elem]->element_count ;
	}
}

SASL_VISIT_DEF_UNIMPL( unary_expression );
SASL_VISIT_DEF_UNIMPL( cast_expression );
SASL_VISIT_DEF_UNIMPL( expression_list );
SASL_VISIT_DEF_UNIMPL( cond_expression );
SASL_VISIT_DEF_UNIMPL( index_expression );

SASL_VISIT_DEF( member_expression ){
	EFLIB_UNREF_PARAM(data);

	visit_child(v.expr);
	node_context* agg_ctxt = node_ctxt( v.expr );
	assert( agg_ctxt );
	
	// Aggregated value
	node_semantic* tisi = sem_->get_semantic(v.expr);
	node_context* ctxt = node_ctxt(v, true);

	if( tisi->ty_proto()->is_builtin() ){
		// Swizzle or write mask
		uint32_t masks = sem_->get_semantic(&v)->swizzle();
		value_t agg_value = agg_ctxt->node_value;
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
			node_context* mem_ctxt = cgllvm_impl::node_ctxt( mem_sym->associated_node() );
			assert( mem_ctxt );
			ctxt->node_value = mem_ctxt->node_value;
			ctxt->node_value.parent( agg_ctxt->node_value );
			ctxt->node_value.abi( agg_ctxt->node_value.abi() );
		}
	}
}

SASL_VISIT_DEF( variable_expression ){
	// TODO Referenced symbol must be evaluated in semantic analysis stages.
	symbol* sym = find_symbol(v.var_name->str);
	assert(sym);
	
	// var_si is not null if sym is global value( sv_none is available )
	sv_layout* var_si = abii->input_sv_layout(sym);

	node_context* varctxt = node_ctxt( sym->associated_node() );
	node_context* ctxt = node_ctxt(v, true);

	if( var_si ){
		// TODO global only available in entry function.
		assert( fn().fn == entry_fn );
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
	EFLIB_UNREF_PARAM(data);

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
	service()->if_cond_end( cgllvm_impl::node_ctxt(v.cond)->node_value );
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
	any child_ctxt_init = *data;
	any child_ctxt;

	service()->while_beg();
	
	insert_point_t cond_beg = service()->new_block( "while.cond", true );
	service()->while_cond_beg();
	visit_child( v.cond );
	tid_t cond_tid = sem_->get_semantic(v.cond)->tid();
	tid_t bool_tid = sem_->pety()->get( builtin_types::_boolean );
	if( cond_tid != bool_tid ){
		caster->cast(sem_->pety()->get_proto(bool_tid), v.cond.get());
	}
	service()->while_cond_end( cgllvm_impl::node_ctxt(v.cond)->node_value );
	value_t joinable = service()->joinable();
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
	service()->jump_cond( joinable, body_beg, while_end );

	service()->set_insert_point( body_end );
	service()->jump_to( cond_beg );

	service()->set_insert_point( while_end );
}
SASL_VISIT_DEF( dowhile_statement )
{
	any child_ctxt_init = *data;
	any child_ctxt;

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
	service()->do_cond_end( cgllvm_impl::node_ctxt(v.cond)->node_value );
	value_t joinable = service()->joinable();
	insert_point_t cond_end = service()->insert_point();

	insert_point_t while_end = service()->new_block( "dowhile.end", true );
	service()->do_end();

	// Fill back
	service()->set_insert_point( body_end );
	service()->jump_to( cond_beg );

	service()->set_insert_point( cond_end );
	service()->jump_cond( joinable, body_beg, while_end );

	service()->set_insert_point( while_end );
}

SASL_VISIT_DEF( for_statement )
{
	EFLIB_UNREF_PARAM(data);
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
	value_t cond_value;
	if( v.cond ){ 
		visit_child( v.cond );
		cond_value = cgllvm_impl::node_ctxt( v.cond, false )->node_value;
	}
	service()->for_cond_end( cond_value );
	value_t joinable = service()->joinable();
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
	service()->jump_cond( joinable, body_beg, for_end );

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
	any child_ctxt_init = *data;
	any child_ctxt;

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
	create_entries();
}

void add_type_ref( Type* ty, vector<Type*>& tys )
{
	tys.push_back( PointerType::getUnqual( ty ) );
}

SASL_SPECIFIC_VISIT_DEF( create_fnsig, function_type )
{
	if( !entry_fn && abii->is_entry( sem_->get_symbol(&v) ) ){

		boost::any child_ctxt;

		vector<Type*> param_types;

		add_type_ref( entry_structs[su_stream_in], param_types );
		add_type_ref( entry_structs[su_buffer_in], param_types );
		add_type_ref( entry_structs[su_stream_out], param_types );
		add_type_ref( entry_structs[su_buffer_out], param_types );

		FunctionType* fntype = FunctionType::get( Type::getVoidTy( cgllvm_impl::context() ), param_types, false );
		Function* fn = Function::Create( fntype, Function::ExternalLinkage, sem_->get_symbol(&v)->mangled_name(), cgllvm_impl::module() );
		entry_fn = fn;
		// entry_sym = v.symbol().get();

		node_context* ctxt = node_ctxt(v, true);
		ctxt->function_scope = ctxt_->create_cg_function();
		ctxt->function_scope->fn = fn;
		ctxt->function_scope->fnty = &v;
		ctxt->function_scope->cg = service();
	} else {
		parent_class::create_fnsig(v, data);
	}
}
SASL_SPECIFIC_VISIT_DEF( create_fnargs, function_type )
{
	node_context* ctxt = node_ctxt(v);
	Function* fn = ctxt->function_scope->fn;

	if( abii->is_entry( sem_->get_symbol(&v) ) ){
		// Create entry arguments.
		Function::arg_iterator arg_it = fn->arg_begin();

		arg_it->setName( ".arg.stri" );
		entry_values[su_stream_in] = service()->create_value( builtin_types::none, arg_it, vkind_ref, abi_package );
		++arg_it;
		arg_it->setName( ".arg.bufi" );
		entry_values[su_buffer_in] = service()->create_value( builtin_types::none, arg_it, vkind_ref, abi_c );
		++arg_it;
		arg_it->setName( ".arg.stro" );
		entry_values[su_stream_out] = service()->create_value( builtin_types::none, arg_it, vkind_ref, abi_package );
		++arg_it;
		arg_it->setName( ".arg.bufo" );
		entry_values[su_buffer_out] = service()->create_value( builtin_types::none, arg_it, vkind_ref, abi_c );
		++arg_it;

		// Create virtual arguments
		create_virtual_args(v, data);
	} else {
		parent_class::create_fnargs(v, data);
	}
}
SASL_SPECIFIC_VISIT_DEF( create_virtual_args, function_type ){
	EFLIB_UNREF_PARAM(data);

	service()->new_block( ".init.vargs", true );
	BOOST_FOREACH( shared_ptr<parameter> const& par, v.params ){
		visit_child( par->param_type );
		node_semantic* par_ssi = sem_->get_semantic(par);

		node_context* pctxt = node_ctxt( par, true );

		// Create local variable for 'virtual argument' and 'virtual result'.
		if( par_ssi->ty_proto()->is_builtin() ){
			// Virtual args for built in typed argument.

			// Get Value from semantic.
			// Store value to local variable.
			salviar::semantic_value const& par_sem = par_ssi->semantic_value_ref();
			assert( par_sem != salviar::sv_none );
			sv_layout* psi = abii->input_sv_layout( par_sem );

			builtin_types hint = par_ssi->ty_proto()->tycode;
			pctxt->node_value = service()->create_variable( hint, service()->param_abi(false), par->name->str );
			pctxt->node_value.store( layout_to_value(psi) );
		} else {
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
	EFLIB_UNREF_PARAM(data);

	if( service()->fn().fn == entry_fn ){
		visit_child( v.jump_expr );

		// Copy result.
		value_t ret_value = cgllvm_impl::node_ctxt( v.jump_expr )->node_value;

		if( ret_value.hint() != builtin_types::none ){
			node_semantic* ret_ssi = sem_->get_semantic(service()->fn().fnty);
			sv_layout* ret_si = abii->output_sv_layout( ret_ssi->semantic_value_ref() );
			assert( ret_si );
			layout_to_value(ret_si).store( ret_value );
		} else {
			shared_ptr<struct_type> ret_struct = service()->fn().fnty->retval_type->as_handle<struct_type>();
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
	EFLIB_ASSERT_UNIMPLEMENTED();
}
SASL_SPECIFIC_VISIT_DEF( visit_break	, jump_statement ){
	service()->break_();
}

SASL_SPECIFIC_VISIT_DEF( bin_logic, binary_expression ){
	EFLIB_ASSERT_UNIMPLEMENTED();
}
value_t cgllvm_simd::layout_to_value( sv_layout* svl )
{
	builtin_types bt = to_builtin_types( svl->value_type );
	value_t ret = service()->emit_extract_ref( entry_values[svl->usage], svl->physical_index );
	ret.hint( to_builtin_types( svl->value_type ) );
	return ret;
}

END_NS_SASL_CODE_GENERATOR();