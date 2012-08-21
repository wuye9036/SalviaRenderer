#include <sasl/include/code_generator/llvm/cgllvm_impl.imp.h>

#include <sasl/include/code_generator/llvm/utility.h>
#include <sasl/include/code_generator/llvm/cgllvm_caster.h>
#include <sasl/include/code_generator/llvm/cgllvm_module_impl.h>
#include <sasl/include/semantic/semantics.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/semantic/caster.h>
#include <sasl/include/semantic/name_mangler.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/expression.h>
#include <sasl/include/syntax_tree/statement.h>
#include <sasl/include/syntax_tree/node.h>
#include <sasl/include/syntax_tree/program.h>
#include <sasl/enums/builtin_types.h>
#include <sasl/enums/enums_utility.h>

#include <eflib/include/diagnostics/assert.h>
#include <eflib/include/utility/scoped_value.h>
#include <eflib/include/utility/polymorphic_cast.h>
#include <eflib/include/utility/unref_declarator.h>

#include <eflib/include/platform/disable_warnings.h>
#include <llvm/DerivedTypes.h>
#include <llvm/Target/TargetData.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/Constants.h>
#include <llvm/Support/TargetSelect.h>
#include <eflib/include/platform/enable_warnings.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/foreach.hpp>
#include <boost/utility.hpp>
#include <boost/format.hpp>
#include <eflib/include/platform/boost_end.h>

#include <vector>

using namespace sasl::syntax_tree;
using namespace sasl::semantic;
using namespace llvm;
using namespace sasl::utility;
using namespace sasl::common;

using eflib::polymorphic_cast;
using eflib::scoped_value;

using boost::bind;
using boost::addressof;
using boost::format;

using std::vector;

#define SASL_VISITOR_TYPE_NAME cgllvm_impl

BEGIN_NS_SASL_CODE_GENERATOR();

llvm::DefaultIRBuilder* cgllvm_impl::builder() const
{
	return llvm_mod_->builder();
}

llvm::LLVMContext& cgllvm_impl::context() const
{
	return llvm_mod_->llvm_context();
}

llvm::Module* cgllvm_impl::module() const
{
	return llvm_mod_->llvm_module();
}

node_context* cgllvm_impl::node_ctxt( node const* n, bool create_if_need /*= false */ )
{
	if(!create_if_need)
	{
		return ctxt_->get_node_context(n);
	}
	else
	{
		return ctxt_->get_or_create_node_context(n);
	}
}

shared_ptr<cgllvm_module> cgllvm_impl::generated_module() const
{
	return llvm_mod_;
}

bool cgllvm_impl::generate( shared_ptr<module_semantic> const& mod, abi_info const* abii )
{
	sem_ = mod;
	this->abii = abii;

	if(sem_){
		assert( sem_->root_symbol() );
		assert( sem_->get_program() );
		sem_->get_program()->accept( this, NULL );
		return true;
	}

	return false;
}

cgllvm_impl::~cgllvm_impl()
{
	if( service_ )
	{
		delete service_;
		service_ = NULL;
	}
	
	// if( target_data ){ delete target_data; }
}

symbol* cgllvm_impl::find_symbol(std::string const& str)
{
	return current_symbol_->find(str);
}  

function_t* cgllvm_impl::get_function( std::string const& name ) const
{
	symbol* callee_sym = sem_->root_symbol()->find_overloads(name)[0];
	node_context* callee_ctxt = const_cast<cgllvm_impl*>(this)->node_ctxt( callee_sym->associated_node() );
	return callee_ctxt->function_scope;
}

cgllvm_impl::cgllvm_impl()
	: abii(NULL), target_data(NULL), service_(NULL)
	, semantic_mode_(false), msc_compatible_(false), current_cg_type_(NULL)
	, parent_struct_(NULL), block_(NULL), current_symbol_(NULL), variable_to_initialize_(NULL)
{
}

void cgllvm_impl::visit_child( sasl::syntax_tree::node* child )
{
	child->accept(this, NULL);
}

SASL_VISIT_DEF( variable_expression ){
	EFLIB_UNREF_DECLARATOR(data);

	symbol* var_sym = current_symbol_->find( v.var_name->str );
	assert(var_sym);
	
	node* var_node = var_sym->associated_node();
	assert(var_node);
	
	node_context* var_ctxt = node_ctxt(var_node, false);
	assert(var_ctxt);

	node_context* ctxt = node_ctxt(v, true);
	ctxt->node_value			= var_ctxt->node_value;
	ctxt->ty					= var_ctxt->ty;
	ctxt->is_semantic_mode	= var_ctxt->is_semantic_mode;
}

SASL_VISIT_DEF( binary_expression ){
	EFLIB_UNREF_DECLARATOR(data);

	if( v.op == operators::logic_and || v.op == operators::logic_or ){
		bin_logic( v, data );
	} else {
		visit_child(v.left_expr);
		visit_child(v.right_expr);

		if(/**/v.op == operators::assign
			|| v.op == operators::add_assign
			|| v.op == operators::sub_assign
			|| v.op == operators::mul_assign
			|| v.op == operators::div_assign
			|| v.op == operators::mod_assign
			|| v.op == operators::lshift_assign
			|| v.op == operators::rshift_assign
			|| v.op == operators::bit_and_assign
			|| v.op == operators::bit_or_assign
			|| v.op == operators::bit_xor_assign
			)
		{
			bin_assign( v, data );
		}
		else
		{
			std::string op_name = operator_name(v.op);

			node_semantic* larg_tsi = sem_->get_semantic(v.left_expr);
			node_semantic* rarg_tsi = sem_->get_semantic(v.right_expr);

			vector<expression*> args;
			args.push_back( v.left_expr.get() );
			args.push_back( v.right_expr.get() );

			symbol::symbol_array overloads = current_symbol_->find_overloads(op_name, caster.get(), args);
			EFLIB_ASSERT( overloads.size() == 1, "No or more an one overloads." );

			function_type* op_proto = dynamic_cast<function_type*>( overloads[0]->associated_node() );

			node_semantic* p0_tsi = sem_->get_semantic(op_proto->params[0]);
			node_semantic* p1_tsi = sem_->get_semantic(op_proto->params[1]);;

			// cast value type to match proto type.
			if( p0_tsi->tid() != larg_tsi->tid() ){
				if( !node_ctxt( p0_tsi->ty_proto() ) ){
					visit_child(op_proto->params[0]->param_type);
				}
				caster->cast( p0_tsi->ty_proto(), v.left_expr.get() );
			}
			if( p1_tsi->tid() != rarg_tsi->tid() ){
				if( !node_ctxt( p1_tsi->ty_proto() ) ){
					visit_child(op_proto->params[1]->param_type);
				}
				caster->cast( p1_tsi->ty_proto(), v.right_expr.get() );
			}

			// use type-converted value to generate code.
			value_t lval = node_ctxt(v.left_expr)->node_value;
			value_t rval = node_ctxt(v.right_expr)->node_value;

			value_t retval;

			builtin_types lbtc = p0_tsi->ty_proto()->tycode;
			builtin_types rbtc = p1_tsi->ty_proto()->tycode;

			if( v.op == operators::add ){
				retval = service()->emit_add(lval, rval);
			} else if ( v.op == operators::mul ) {
				retval = service()->emit_mul_comp(lval, rval);
			} else if ( v.op == operators::sub ) {
				retval = service()->emit_sub(lval, rval);
			} else if( v.op == operators::div ){
				retval = service()->emit_div(lval, rval);
			} else if( v.op == operators::mod ){
				retval = service()->emit_mod(lval, rval);
			} else if( v.op == operators::left_shift ) {
				retval = service()->emit_lshift(lval, rval);
			} else if( v.op == operators::right_shift ) {
				retval = service()->emit_rshift(lval, rval);
			} else if( v.op == operators::bit_and ) {
				retval = service()->emit_bit_and(lval, rval);
			} else if( v.op == operators::bit_or ) {
				retval = service()->emit_bit_or(lval, rval);
			} else if( v.op == operators::bit_xor ) {
				retval = service()->emit_bit_xor(lval, rval);
			} else if( v.op == operators::less ) {
				retval = service()->emit_cmp_lt(lval, rval);
			} else if( v.op == operators::less_equal ){
				retval = service()->emit_cmp_le(lval, rval);
			} else if( v.op == operators::equal ){
				retval = service()->emit_cmp_eq(lval, rval);
			} else if( v.op == operators::greater_equal ){
				retval = service()->emit_cmp_ge(lval, rval);
			} else if( v.op == operators::greater ){
				retval = service()->emit_cmp_gt(lval, rval);	
			} else if( v.op == operators::not_equal ){
				retval = service()->emit_cmp_ne(lval, rval);
			} else {
				EFLIB_ASSERT_UNIMPLEMENTED0( ( operators::to_name(v.op) + " not be implemented." ).c_str() );
			}

			assert(retval.hint() == sem_->get_semantic(op_proto)->ty_proto()->tycode);

			node_ctxt(v, true)->node_value = retval;
		}
	}
}

SASL_VISIT_DEF( constant_expression ){
	EFLIB_UNREF_DECLARATOR(data);

	node_semantic* const_sem = sem_->get_semantic(&v);
	if( ! node_ctxt( const_sem->ty_proto() ) ){
		visit_child( const_sem->ty_proto()->as_handle() );
	}

	node_context* const_ctxt = node_ctxt( const_sem->ty_proto() );

	cg_type* ty = const_ctxt->ty;
	assert( ty );

	value_t val;
	builtin_types const_tycode = const_sem->value_builtin_type();
	if( const_tycode == builtin_types::_sint64 ) {
		val = service()->create_constant_scalar( static_cast<int64_t>( const_sem->const_signed() ),		ty, ty->hint() );
	} else if ( const_tycode == builtin_types::_sint32 ) {
		val = service()->create_constant_scalar( static_cast<int32_t>( const_sem->const_signed() ),		ty, ty->hint() );
	} else if ( const_tycode == builtin_types::_uint64 ) {
		val = service()->create_constant_scalar( static_cast<uint64_t>( const_sem->const_unsigned() ),	ty, ty->hint() );
	} else if ( const_tycode == builtin_types::_uint32 ) {
		val = service()->create_constant_scalar( static_cast<uint32_t>( const_sem->const_unsigned() ),	ty, ty->hint() );
	} else if ( const_tycode == builtin_types::_float ) {
		val = service()->create_constant_scalar( static_cast<float>( const_sem->const_double() ),		ty, ty->hint() );
	} else {
		EFLIB_ASSERT_UNIMPLEMENTED();
	}

	node_ctxt(v, true)->node_value = val;
}

SASL_VISIT_DEF( call_expression ){
	EFLIB_UNREF_DECLARATOR(data);

	node_semantic* csi = sem_->get_semantic(&v);
	if( csi->is_function_pointer() ){
		visit_child( v.expr );
		EFLIB_ASSERT_UNIMPLEMENTED();
	} else {
		// Get LLVM Function
		symbol* fn_sym = csi->overloaded_function();
		function_type* proto = polymorphic_cast<function_type*>( fn_sym->associated_node() );
		
		vector<value_t> args;
		for( size_t i_arg = 0; i_arg < v.args.size(); ++i_arg )
		{
			visit_child( v.args[i_arg] );

			node_semantic* arg_tisi = sem_->get_semantic(v.args[i_arg]);
			node_semantic* par_tisi = sem_->get_semantic(proto->params[i_arg]);
			
			if( par_tisi->tid() != arg_tisi->tid() )
			{
				if( !node_ctxt( par_tisi->ty_proto() ) )
				{
					visit_child( proto->params[i_arg]->param_type );
				}
				caster->cast( par_tisi->ty_proto(), v.args[i_arg].get() );
			}

			node_context* arg_ctxt = node_ctxt(v.args[i_arg], false);
			args.push_back(arg_ctxt->node_value);
		}
		
		function_t* fn	= service()->fetch_function(proto);
		value_t	rslt	= service()->emit_call(*fn, args);

		node_context* expr_ctxt = node_ctxt( v, true );
		expr_ctxt->node_value = rslt;
		expr_ctxt->ty = fn->get_return_ty();
	}
}

SASL_VISIT_DEF( index_expression )
{
	EFLIB_UNREF_DECLARATOR(data);

	visit_child( v.expr );
	visit_child( v.index_expr );
	node_context* expr_ctxt  = node_ctxt(v.expr);
	node_context* index_ctxt = node_ctxt(v.index_expr);
	assert( expr_ctxt && index_ctxt );

	node_context* ret_ctxt = node_ctxt( v, true );
	ret_ctxt->node_value	= service()->emit_extract_elem( expr_ctxt->node_value, index_ctxt->node_value );
	ret_ctxt->ty			= service()->create_ty( sem_->get_semantic(&v)->ty_proto() );
}

SASL_VISIT_DEF( builtin_type ){
	EFLIB_UNREF_DECLARATOR(data);

	node_semantic* tisi = sem_->get_semantic(&v);
	node_context* ctxt = ctxt_->get_or_create_node_context(&v);
	if( ctxt->ty ) { return; }

	node_context* proto_ctxt = node_ctxt( tisi->ty_proto(), true );
	if( !proto_ctxt->ty )
	{
		cg_type* bt_tyinfo = service()->create_ty( tisi->ty_proto() );
		assert( bt_tyinfo );
		proto_ctxt->ty = bt_tyinfo;

		std::string tips = v.tycode.name() + std::string(" was not supported yet.");
		EFLIB_ASSERT_AND_IF( proto_ctxt->ty, tips.c_str() ){
			return;
		}
	}
	*ctxt = *proto_ctxt;
}

SASL_VISIT_DEF( parameter ){
	EFLIB_UNREF_DECLARATOR(data);

	visit_child( v.param_type );

	if( v.init ){ visit_child( v.init ); }
	
	node_context* init_ctxt	= node_ctxt(v.param_type);
	node_context* ctxt		= node_ctxt(v, true);

	ctxt->node_value= init_ctxt->node_value;
	ctxt->ty			= init_ctxt->ty;
}

// Generate normal function code.
SASL_VISIT_DEF( function_type )
{
	EFLIB_UNREF_DECLARATOR(data);
	SYMBOL_SCOPE( sem_->get_symbol(&v) );

	node_context* fn_ctxt = node_ctxt(v, true);

	if(!fn_ctxt->function_scope)
	{
		create_fnsig(v, NULL);
	}

	if ( v.body )
	{
		CGS_FUNCTION_SCOPE(fn_ctxt->function_scope);

		service()->function_beg();
		service()->fn().allocation_block( service()->new_block(".alloc", true) );
		create_fnargs(v, NULL);
		create_fnbody(v, NULL);
		service()->function_end();
	}
}

SASL_VISIT_DEF( struct_type ){
	EFLIB_UNREF_DECLARATOR(data);

	// Create context.
	// Declarator visiting need parent information.
	node_context* ctxt = node_ctxt(v, true);

	// A struct is visited at definition type.
	// If the visited again, it must be as an alias_type.
	// So return environment directly.
	if( ctxt->ty ) { return; }

	// Init data.
	STRUCT_SCOPE(ctxt);
	BOOST_FOREACH( shared_ptr<declaration> const& decl, v.decls ){
		visit_child(decl);
	}

	ctxt->ty = service()->create_ty( sem_->get_semantic(&v)->ty_proto() );
}

SASL_VISIT_DEF( array_type )
{
	EFLIB_UNREF_DECLARATOR(data);
	node_context* ctxt = node_ctxt(v, true);
	
	if( ctxt->ty ){ return; }
	visit_child(v.elem_type);

	ctxt->ty = service()->create_ty( sem_->get_semantic(&v)->ty_proto() );
}

SASL_VISIT_DEF( variable_declaration ){
	EFLIB_UNREF_DECLARATOR(data);

	// Visit type info
	visit_child( v.type_info );
	node_context* ty_ctxt = node_ctxt(v.type_info);
	node_context* ctxt = node_ctxt(v, true);
	
	TYPE_SCOPE(ty_ctxt->ty);
	BOOST_FOREACH( shared_ptr<declarator> const& dclr, v.declarators )
	{
		visit_child(dclr);
	}

	ctxt->ty = ty_ctxt->ty;
	ctxt->declarator_count = static_cast<int>( v.declarators.size() );
}

SASL_VISIT_DEF( declarator ){
	EFLIB_UNREF_DECLARATOR(data);

	// local or member.
	// TODO TBD: Support member function and nested structure ?
	if( service()->in_function() ){
		visit_local_declarator(v, NULL);
	} else if(parent_struct_){
		visit_member_declarator(v, NULL);
	} else {
		visit_global_declarator(v, NULL);
	}
}

SASL_VISIT_DEF( expression_initializer ){
	EFLIB_UNREF_DECLARATOR(data);

	visit_child( v.init_expr );

	node_semantic* init_tsi = sem_->get_semantic(&v);
	node_semantic* var_tsi = sem_->get_semantic(variable_to_initialize_);

	if( init_tsi->tid() != var_tsi->tid() )
	{
		caster->cast( var_tsi->ty_proto(), v.init_expr.get() );
	}

	*node_ctxt(v, true) = *node_ctxt(v.init_expr, false);
}

SASL_VISIT_DEF( expression_statement )
{
	EFLIB_UNREF_DECLARATOR(data);
	visit_child( v.expr );
}

SASL_VISIT_DEF( declaration_statement ){
	EFLIB_UNREF_DECLARATOR(data);

	BOOST_FOREACH( shared_ptr<declaration> const& decl, v.decls )
	{
		visit_child( decl );
	}
}

SASL_VISIT_DEF( jump_statement )
{
	EFLIB_UNREF_DECLARATOR(data);

	if (v.jump_expr){
		visit_child( v.jump_expr );
	}

	if ( v.code == jump_mode::_return ){
		visit_return(v, NULL);
	} else if ( v.code == jump_mode::_continue ){
		visit_continue(v, NULL);
	} else if ( v.code == jump_mode::_break ){
		visit_break(v, NULL);
	}

	// Restart a new block for sealing the old block.
	service()->new_block(".restart", true);
}

SASL_VISIT_DEF( program )
{	
	EFLIB_UNREF_DECLARATOR(data);

	// Create module.
	assert( !llvm_mod_ );
	llvm_mod_.reset( new cgllvm_module_impl() );
	ctxt_ = module_context::create();

	// Initialization.
	llvm_mod_->create_llvm_module(v.name);
	llvm_mod_->set_semantic(sem_);
	llvm_mod_->set_context(ctxt_);

	service()->initialize( llvm_mod_.get(), ctxt_.get(), sem_.get() );

	typedef node_context* (get_context_native_fn) (node const*);
	boost::function<get_context_native_fn> get_context
		= boost::bind(&module_context::get_node_context, ctxt_.get(), _1);

	typedef tynode* (get_proto_native_fn)(tid_t);
	boost::function<get_proto_native_fn> get_proto
		= boost::bind(&pety_t::get_proto, sem_->pety(), _1);

	typedef node_semantic* (get_semantic_native_fn)(node const*);
	typedef node_semantic* (module_semantic::*get_semantic_mem_fn)(node const*) const;
	boost::function<get_semantic_native_fn> get_semantic
		= boost::bind(
			static_cast<get_semantic_mem_fn>(&module_semantic::get_semantic),
			sem_.get(), _1 );

	caster = create_cgllvm_caster( get_context, get_semantic, get_proto, service() );
	add_builtin_casts( caster, sem_->pety() );
	
	process_intrinsics(v, NULL);

	// Some other initializations.
	before_decls_visit(v, NULL);

	// visit declarations
	for( vector< shared_ptr<declaration> >::iterator
		it = v.decls.begin(); it != v.decls.end(); ++it )
	{
		visit_child(*it);
	}
}

SASL_SPECIFIC_VISIT_DEF( before_decls_visit, program )
{
	EFLIB_UNREF_DECLARATOR(data);
	EFLIB_UNREF_DECLARATOR(v);

	TargetMachine* tm = EngineBuilder(module()).selectTarget();
	target_data = tm->getTargetData();
}

SASL_SPECIFIC_VISIT_DEF( visit_member_declarator, declarator ){
	EFLIB_UNREF_DECLARATOR(data);

	assert(current_cg_type_);

	// Needn't process init expression now.
	node_semantic* sem = sem_->get_semantic(&v);
	node_context* ctxt = node_ctxt(v, true);
	ctxt->ty = current_cg_type_;
	ctxt->node_value = service()->create_value(current_cg_type_, NULL, vkind_swizzle, abi_unknown );
	ctxt->node_value.index( sem->member_index() );
}

SASL_SPECIFIC_VISIT_DEF( visit_global_declarator, declarator )
{
	EFLIB_UNREF_DECLARATOR(data);
	EFLIB_UNREF_DECLARATOR(v);
}

SASL_SPECIFIC_VISIT_DEF( visit_local_declarator , declarator ){
	EFLIB_UNREF_DECLARATOR(data);

	node_context* ctxt = node_ctxt(v, true);

	ctxt->ty = current_cg_type_;
	ctxt->node_value = service()->create_variable( ctxt->ty, local_abi( sem_->get_semantic(&v)->msc_compatible() ), v.name->str );

	if ( v.init )
	{
		VARIABLE_TO_INIT_SCOPE(&v);
		visit_child(v.init);
		ctxt->node_value.store( node_ctxt(v.init)->node_value );
	}
}

SASL_SPECIFIC_VISIT_DEF( create_fnsig, function_type ){
	EFLIB_UNREF_DECLARATOR(data);

	// Generate return type node.
	visit_child( v.retval_type );
	assert( node_ctxt(v.retval_type)->ty );

	// Generate parameters.
	node_context* ctxt = node_ctxt(v);

	BOOST_FOREACH( shared_ptr<parameter> const& par, v.params )
	{
		visit_child( par );
	}

	ctxt->function_scope = service()->fetch_function(&v);
}
SASL_SPECIFIC_VISIT_DEF( create_fnargs, function_type ){

	EFLIB_UNREF_DECLARATOR(data);

	// Register arguments names.
	assert( service()->fn().arg_size() == v.params.size() );

	service()->fn().return_name( ".ret" );
	size_t i_arg = 0;
	BOOST_FOREACH( shared_ptr<parameter> const& par, v.params )
	{
		node_context* par_ctxt = node_ctxt( par );
		service()->fn().arg_name( i_arg, sem_->get_symbol( par.get() )->unmangled_name() );
		par_ctxt->node_value = service()->fn().arg(i_arg++);
	}
}

SASL_SPECIFIC_VISIT_DEF( create_fnbody, function_type ){
	EFLIB_UNREF_DECLARATOR(data);

	service()->new_block(".body", true);
	visit_child( v.body );

	service()->clean_empty_blocks();
}

SASL_SPECIFIC_VISIT_DEF( visit_return, jump_statement ){
	EFLIB_UNREF_DECLARATOR(data);

	if ( !v.jump_expr ){
		service()->emit_return();
	} else {
		shared_ptr<tynode> fn_retty = service()->fn().fnty->retval_type;
		tid_t fret_tid = sem_->get_semantic(fn_retty)->tid();
		tid_t expr_tid = sem_->get_semantic(v.jump_expr)->tid();
		if( fret_tid != expr_tid )
		{
			caster->cast( fn_retty, v.jump_expr );
		}
		service()->emit_return( node_ctxt(v.jump_expr)->node_value, service()->param_abi( service()->fn().c_compatible ) );
	}
}

/* Make binary assignment code.
*    Note: Right argument is assignee, and left argument is value.
*/
SASL_SPECIFIC_VISIT_DEF( bin_assign, binary_expression ){
	EFLIB_UNREF_DECLARATOR(data);

	std::string op_name = operator_name(v.op);

	node_semantic* larg_tsi = sem_->get_semantic(v.left_expr);

	vector<expression*> args;
	args.push_back( v.left_expr.get() );
	args.push_back( v.right_expr.get() );

	symbol::symbol_array overloads = current_symbol_->find_overloads(op_name, caster.get(), args);
	EFLIB_ASSERT( overloads.size() == 1, "No or more an one overloads." );

	function_type* op_proto = polymorphic_cast<function_type*>( overloads[0]->associated_node() );

	node_semantic* p0_tsi = sem_->get_semantic(op_proto->params[0]);
	if( p0_tsi->tid() != larg_tsi->tid() )
	{
		if( !node_ctxt( p0_tsi->ty_proto() ) )
		{
			visit_child( op_proto->params[0]->param_type );
		}
		caster->cast( p0_tsi->ty_proto(), v.left_expr.get() );
	}

	// Evaluated by visit(binary_expression)
	node_context* lctxt = node_ctxt( v.left_expr );
	node_context* rctxt = node_ctxt( v.right_expr );

	value_t val;
	/**/ if( v.op == operators::add_assign )
	{
		val = service()->emit_add( rctxt->node_value, lctxt->node_value );
	}
	else if( v.op == operators::sub_assign )
	{
		val = service()->emit_sub( rctxt->node_value, lctxt->node_value );
	}
	else if( v.op == operators::mul_assign )
	{
		val = service()->emit_mul_comp( rctxt->node_value, lctxt->node_value );
	}
	else if( v.op == operators::div_assign )
	{
		val = service()->emit_div( rctxt->node_value, lctxt->node_value );
	}
	else if( v.op == operators::mod_assign )
	{
		val = service()->emit_mod( rctxt->node_value, lctxt->node_value );
	}
	else if( v.op == operators::lshift_assign )
	{
		val = service()->emit_lshift( rctxt->node_value, lctxt->node_value );
	}
	else if( v.op == operators::rshift_assign )
	{
		val = service()->emit_rshift( rctxt->node_value, lctxt->node_value );
	}
	else if( v.op == operators::bit_and_assign )
	{
		val = service()->emit_bit_and( rctxt->node_value, lctxt->node_value );
	}
	else if( v.op == operators::bit_or_assign )
	{
		val = service()->emit_bit_or( rctxt->node_value, lctxt->node_value );
	}
	else if( v.op == operators::bit_xor_assign )
	{
		val = service()->emit_bit_xor( rctxt->node_value, lctxt->node_value );
	}
	else if( v.op == operators::sub_assign )
	{
		val = service()->emit_add( rctxt->node_value, lctxt->node_value );
	}
	else
	{
		assert( v.op == operators::assign );
		val = lctxt->node_value;
	}

	rctxt->node_value.store(val);

	node_context* ctxt = node_ctxt(v, true);
	*ctxt = *rctxt;
}

SASL_SPECIFIC_VISIT_DEF( process_intrinsics, program )
{
	EFLIB_UNREF_DECLARATOR(data);
	EFLIB_UNREF_DECLARATOR(v);

	service()->register_external_intrinsic();

	vector<symbol*> const& intrinsics = sem_->intrinsics();

	BOOST_FOREACH(symbol* intr, intrinsics)
	{
		function_type* intr_fn = polymorphic_cast<function_type*>( intr->associated_node() );
		node_semantic* intrin_ssi = sem_->get_semantic(intr_fn);
		bool external = intrin_ssi->is_external();

		// If intrinsic is not invoked, we don't generate code for it.
		if( !intrin_ssi->is_invoked() && !external ){ continue;	}

		visit_child(intr_fn);
		// Deal with external functions. External function has nobody.
		if ( external ){ continue; }

		node_context* intrinsic_ctxt = node_ctxt( intr_fn, false );
		assert( intrinsic_ctxt );

		service()->push_fn(intrinsic_ctxt->function_scope);
		cg_scope_guard<void> pop_fn_on_exit( bind( &cg_service::pop_fn, service() ) );

		service()->fn().allocation_block( service()->new_block(".alloc", true) );
		insert_point_t ip_body = service()->new_block( ".body", true );

		// Parse Parameter Informations
		vector<tynode*> par_tys;
		vector<builtin_types> par_tycodes;
		vector<node_context*> par_ctxts;

		BOOST_FOREACH( shared_ptr<parameter> const& par, intr_fn->params )
		{
			par_tys.push_back( sem_->get_semantic(par)->ty_proto() );
			assert( par_tys.back() );
			par_tycodes.push_back( par_tys.back()->tycode );
			par_ctxts.push_back( node_ctxt(par, false) );
			assert( par_ctxts.back() );
		}

		// cg_type* result_ty = service()->fn().get_return_ty();
		
		service()->fn().inline_hint();

		// Process Intrinsic
		if( intr->unmangled_name() == "mul" )
		{
			assert( par_tys.size() == 2 );

			// Set Argument name
			service()->fn().arg_name( 0, ".lhs" );
			service()->fn().arg_name( 1, ".rhs" );

			value_t ret_val = service()->emit_mul_intrin( service()->fn().arg(0), service()->fn().arg(1) );
			service()->emit_return( ret_val, service()->param_abi(false) );

		}
		else if( intr->unmangled_name() == "dot" )
		{
			assert( par_tys.size() == 2 );

			// Set Argument name
			service()->fn().arg_name( 0, ".lhs" );
			service()->fn().arg_name( 1, ".rhs" );

			value_t ret_val = service()->emit_dot( service()->fn().arg(0), service()->fn().arg(1) );
			service()->emit_return( ret_val, service()->param_abi(false) );

		}
		else if ( intr->unmangled_name() == "abs" )
		{
			assert( par_tys.size() == 1 );
			service()->fn().arg_name( 0, ".value" );
			value_t ret_val = service()->emit_abs( service()->fn().arg(0) );
			service()->emit_return( ret_val, service()->param_abi(false) );
		}
		else if ( intr->unmangled_name() == "exp"
			|| intr->unmangled_name() == "exp2"
			|| intr->unmangled_name() == "sin"
			|| intr->unmangled_name() == "cos"
			|| intr->unmangled_name() == "tan"
			|| intr->unmangled_name() == "asin"
			|| intr->unmangled_name() == "acos"
			|| intr->unmangled_name() == "atan"
			|| intr->unmangled_name() == "ceil"
			|| intr->unmangled_name() == "floor"
			|| intr->unmangled_name() == "round"
			|| intr->unmangled_name() == "trunc"
			|| intr->unmangled_name() == "log"
			|| intr->unmangled_name() == "log2"
			|| intr->unmangled_name() == "log10"
			|| intr->unmangled_name() == "rsqrt"
			|| intr->unmangled_name() == "sinh"
			|| intr->unmangled_name() == "cosh"
			|| intr->unmangled_name() == "tanh"
			)
		{
			assert( par_tys.size() == 1 );
			service()->fn().arg_name( 0, ".value" );
			std::string scalar_intrin_name = ( format("sasl.%s.f32") % intr->unmangled_name() ).str();
			value_t ret_val = service()->emit_unary_ps( scalar_intrin_name,service()->fn().arg(0) );
			service()->emit_return( ret_val, service()->param_abi(false) );
		}
		else if ( intr->unmangled_name() == "countbits"
			|| intr->unmangled_name() == "count_bits"
			)
		{
			assert( par_tys.size() == 1 );
			service()->fn().arg_name( 0, ".value" );
			value_t ret_val = service()->emit_unary_ps( "sasl.countbits.u32", service()->fn().arg(0) );
			service()->emit_return( ret_val, service()->param_abi(false) );
		}
		else if ( intr->unmangled_name() == "firstbithigh"
			|| intr->unmangled_name() == "firstbitlow"
			|| intr->unmangled_name() == "reversebits"
			)
		{
			assert( par_tys.size() == 1 );
			service()->fn().arg_name( 0, ".value" );
			value_t ret_val = service()->emit_unary_ps( ( format("sasl.%s.u32") % intr->unmangled_name() ).str(), service()->fn().arg(0) );
			service()->emit_return( ret_val, service()->param_abi(false) );
		}
		else if( intr->unmangled_name() == "ldexp"
			  || intr->unmangled_name() == "pow" )
		{
			assert( par_tys.size() == 2 );
			service()->fn().arg_name( 0, ".lhs" );
			service()->fn().arg_name( 1, ".rhs" );
			std::string scalar_intrin_name = ( format("sasl.%s.f32") % intr->unmangled_name() ).str();
			value_t ret_val = service()->emit_bin_ps_ta_sva( scalar_intrin_name, service()->fn().arg(0), service()->fn().arg(1) );
			service()->emit_return( ret_val, service()->param_abi(false) );
		}
		else if( intr->unmangled_name() == "sqrt" )
		{
			assert( par_tys.size() == 1 );
			service()->fn().arg_name( 0, ".value" );
			value_t ret_val = service()->emit_sqrt( service()->fn().arg(0) );
			service()->emit_return( ret_val, service()->param_abi(false) );
		}
		else if( intr->unmangled_name() == "cross" )
		{
			assert( par_tys.size() == 2 );
			service()->fn().arg_name( 0, ".lhs" );
			service()->fn().arg_name( 1, ".rhs" );
			value_t ret_val = service()->emit_cross( service()->fn().arg(0), service()->fn().arg(1) );
			service()->emit_return( ret_val, service()->param_abi(false) );
		}
		else if ( intr->unmangled_name() == "ddx" || intr->unmangled_name() == "ddy" )
		{
			assert( par_tys.size() == 1 );
			service()->fn().arg_name( 0, ".value" );

			value_t ret_val;
			if( intr->unmangled_name() == "ddx" ){
				ret_val = service()->emit_ddx( service()->fn().arg(0) );
			} else {
				ret_val = service()->emit_ddy( service()->fn().arg(0) );
			}
			service()->emit_return( ret_val, service()->param_abi(false) );
		}
		else if ( intr->unmangled_name() == "tex2D" )
		{
			assert( par_tys.size() == 2 );
			value_t samp = service()->fn().arg(0);
			value_t coord = service()->fn().arg(1);
			value_t ddx = service()->emit_ddx(coord);
			value_t ddy = service()->emit_ddy(coord);
			value_t ret = service()->emit_tex2Dgrad( samp, coord, ddx, ddy );
			service()->emit_return( ret, service()->param_abi(false) );
		}
		else if ( intr->unmangled_name() == "tex2Dgrad" )
		{
			assert( par_tys.size() == 4 );
			value_t ret = service()->emit_tex2Dgrad(
				service()->fn().arg(0),
				service()->fn().arg(1),
				service()->fn().arg(2),
				service()->fn().arg(3)
				);
			service()->emit_return( ret, service()->param_abi(false) );
		}
		else if ( intr->unmangled_name() == "tex2Dlod" )
		{
			assert( par_tys.size() == 2 );
			value_t ret = service()->emit_tex2Dlod( service()->fn().arg(0), service()->fn().arg(1) );
			service()->emit_return( ret, service()->param_abi(false) );
		}
		else if ( intr->unmangled_name() == "tex2Dbias" )
		{
			assert( par_tys.size() == 2 );
			value_t ret = service()->emit_tex2Dbias( service()->fn().arg(0), service()->fn().arg(1) );
			service()->emit_return( ret, service()->param_abi(false) );
		}
		else if( intr->unmangled_name() == "tex2Dproj" )
		{
			assert( par_tys.size() == 2 );
			value_t ret = service()->emit_tex2Dproj( service()->fn().arg(0), service()->fn().arg(1) );
			service()->emit_return( ret, service()->param_abi(false) );
		}
		else if( intr->unmangled_name() == "texCUBE" )
		{
			assert( par_tys.size() == 2 );
			value_t samp = service()->fn().arg(0);
			value_t coord = service()->fn().arg(1);
			value_t ddx = service()->emit_ddx(coord);
			value_t ddy = service()->emit_ddy(coord);
			value_t ret = service()->emit_texCUBEgrad( samp, coord, ddx, ddy );
			service()->emit_return( ret, service()->param_abi(false) );
		}
		else if( intr->unmangled_name() == "texCUBElod" )
		{
			assert( par_tys.size() == 2 );
			value_t ret = service()->emit_texCUBElod( service()->fn().arg(0), service()->fn().arg(1) );
			service()->emit_return( ret, service()->param_abi(false) );
		}
		else if( intr->unmangled_name() == "texCUBEgrad" )
		{
			assert( par_tys.size() == 4 );
			value_t ret = service()->emit_texCUBEgrad(
				service()->fn().arg(0),
				service()->fn().arg(1),
				service()->fn().arg(2),
				service()->fn().arg(3)
				);
			service()->emit_return( ret, service()->param_abi(false) );
		}
		else if( intr->unmangled_name() == "texCUBEbias" )
		{
			assert( par_tys.size() == 2 );
			value_t ret = service()->emit_texCUBEbias( service()->fn().arg(0), service()->fn().arg(1) );
			service()->emit_return( ret, service()->param_abi(false) );
		}
		else if( intr->unmangled_name() == "texCUBEproj" )
		{
			assert( par_tys.size() == 2 );
			value_t ret = service()->emit_texCUBEproj( service()->fn().arg(0), service()->fn().arg(1) );
			service()->emit_return( ret, service()->param_abi(false) );
		}
		else if ( intrin_ssi->is_constructor() )
		{
			function_t& fn = service()->fn();
			for(size_t i = 0; i < fn.arg_size(); ++i)
			{
				char name[4] = ".v0";
				name[2] += (char)i;
				fn.arg_name(i, name);
			}
			cg_type* ret_ty			= fn.get_return_ty();
			builtin_types ret_hint	= ret_ty->hint();
			
			if( is_vector(ret_hint) ){
				value_t ret_v = service()->undef_value( ret_hint, service()->param_abi(false) );

				size_t i_scalar = 0;
				for( size_t i_arg = 0; i_arg < fn.arg_size(); ++i_arg ){
					value_t arg_value = fn.arg(i_arg);
					builtin_types arg_hint = arg_value.hint();
					if( is_scalar(arg_hint) ){
						ret_v = service()->emit_insert_val( ret_v, static_cast<int>(i_scalar), arg_value );
						++i_scalar;
					} else if ( is_vector(arg_hint) ) {
						size_t arg_vec_size = vector_size(arg_hint);
						for( size_t i_scalar_in_arg = 0; i_scalar_in_arg < arg_vec_size; ++i_scalar_in_arg ){
							value_t scalar_value = service()->emit_extract_val( arg_value, static_cast<int>(i_scalar_in_arg) );
							ret_v = service()->emit_insert_val( ret_v, static_cast<int>(i_scalar), scalar_value );
							++i_scalar;
						}
					} else {
						// TODO: Error.
						assert( false );
					}
				}
				service()->emit_return( ret_v, service()->param_abi(false) );
			} else {
				EFLIB_ASSERT_UNIMPLEMENTED();
			}
		}
		else if( intr->unmangled_name() == "asint" || intr->unmangled_name() == "asfloat" || intr->unmangled_name() == "asuint" )
		{
			function_t& fn = service()->fn();
			fn.arg_name(0, "v");
			cg_type* ret_ty = fn.get_return_ty();
			service()->emit_return( service()->cast_bits(fn.arg(0), ret_ty), service()->param_abi(false) );
		}
		else if( intr->unmangled_name() == "fmod" )
		{
			function_t& fn = service()->fn();

			assert( fn.arg_size() == 2 );
			fn.arg_name(0, "lhs");
			fn.arg_name(1, "rhs");
			
			service()->emit_return( service()->emit_mod( fn.arg(0), fn.arg(1) ), service()->param_abi(false) );
		}
		else if( intr->unmangled_name() == "radians" )
		{
			function_t& fn = service()->fn();
			assert( fn.arg_size() == 1 );
			fn.arg_name(0, ".deg");
			float deg2rad = (float)(eflib::PI/180.0f);
			value_t deg2rad_scalar_v = service()->create_constant_scalar(deg2rad, NULL, builtin_types::_float);
			value_t deg2rad_v = service()->create_value_by_scalar( deg2rad_scalar_v, fn.arg(0).tyinfo(), fn.arg(0).tyinfo()->hint() );
			service()->emit_return( service()->emit_mul_comp( deg2rad_v, fn.arg(0) ), service()->param_abi(false) );
		}
		else if( intr->unmangled_name() == "degrees" )
		{
			function_t& fn = service()->fn();
			assert( fn.arg_size() == 1 );
			fn.arg_name(0, ".rad");
			float rad2deg = (float)(180.0f/eflib::PI);
			value_t rad2deg_scalar_v = service()->create_constant_scalar(rad2deg, NULL, builtin_types::_float);
			value_t rad2deg_v = service()->create_value_by_scalar( rad2deg_scalar_v, fn.arg(0).tyinfo(), fn.arg(0).tyinfo()->hint() );
			service()->emit_return( service()->emit_mul_comp( rad2deg_v, fn.arg(0) ), service()->param_abi(false) );
		}
		else if( intr->unmangled_name() == "lerp" )
		{
			function_t& fn = service()->fn();
			assert( fn.arg_size() == 3 );
			fn.arg_name(0, ".s");
			fn.arg_name(1, ".d");
			fn.arg_name(2, ".t");
			value_t diff = service()->emit_sub( fn.arg(1), fn.arg(0) );
			value_t t_diff = service()->emit_mul_comp( diff, fn.arg(2) );
			value_t ret = service()->emit_add(fn.arg(0), t_diff);
			service()->emit_return( ret, service()->param_abi(false) );
		}
		else if( intr->unmangled_name() == "distance" )
		{
			function_t& fn = service()->fn();
			assert( fn.arg_size() == 2 );
			fn.arg_name(0, ".s");
			fn.arg_name(1, ".d");
			value_t diff = service()->emit_sub( fn.arg(1), fn.arg(0) );
			value_t dist_sqr = service()->emit_dot(diff, diff);
			value_t dist = service()->emit_sqrt(dist_sqr);
			service()->emit_return( dist, service()->param_abi(false) );
		}
		else if( intr->unmangled_name() == "dst" )
		{
			function_t& fn = service()->fn();
			assert( fn.arg_size() == 2 );
			fn.arg_name(0, ".sqr");
			fn.arg_name(1, ".inv");
			value_t x2 = service()->create_constant_scalar(1.0f, NULL, builtin_types::_float);
			value_t y0 = service()->emit_extract_val( fn.arg(0), 1 );
			value_t y1 = service()->emit_extract_val( fn.arg(1), 1 );
			value_t z0 = service()->emit_extract_val( fn.arg(0), 2 );
			value_t w1 = service()->emit_extract_val( fn.arg(1), 3 );
			value_t y2 = service()->emit_mul_comp( y0, y1 );
			vector<value_t> elems;
			elems.push_back(x2);
			elems.push_back(y2);
			elems.push_back(z0);
			elems.push_back(w1);
			value_t dest = service()->create_vector(elems, service()->param_abi(false) );
			service()->emit_return( dest, service()->param_abi(false) );
		}
		else if( intr->unmangled_name() == "any" )
		{
			function_t& fn = service()->fn();
			assert( fn.arg_size() == 1 );
			fn.arg_name(0, ".v");
			service()->emit_return( service()->emit_any(fn.arg(0)), service()->param_abi(false) );
		}
		else if( intr->unmangled_name() == "all" )
		{
			function_t& fn = service()->fn();
			assert( fn.arg_size() == 1 );
			fn.arg_name(0, ".v");
			service()->emit_return( service()->emit_all(fn.arg(0)), service()->param_abi(false) );
		}
		else if( intr->unmangled_name() == "length" )
		{
			function_t& fn = service()->fn();
			assert( fn.arg_size() == 1 );
			fn.arg_name(0, ".v");

			value_t length_sqr = service()->emit_dot( fn.arg(0), fn.arg(0) );
			service()->emit_return( service()->emit_sqrt( length_sqr ), service()->param_abi(false) );
		}
		else if( intr->unmangled_name() == "clamp" )
		{
			function_t& fn = service()->fn();
			assert(fn.arg_size() == 3);
			
			fn.arg_name(0, "v0");
			fn.arg_name(1, "v1");
			fn.arg_name(2, "v2");
			
			value_t v     = fn.arg(0);
			value_t min_v = fn.arg(1);
			value_t max_v = fn.arg(2);

			service()->emit_return( service()->emit_clamp(v, min_v, max_v), service()->param_abi(false) );
		}
		else if( intr->unmangled_name() == "isinf" )
		{
			function_t& fn = service()->fn();
			assert(fn.arg_size() == 1);
			fn.arg_name(0, "v");

			service()->emit_return(
				service()->emit_isinf(fn.arg(0)),
				service()->param_abi(false)
				);
		}
		else if( intr->unmangled_name() == "isfinite" )
		{
			function_t& fn = service()->fn();
			assert(fn.arg_size() == 1);
			fn.arg_name(0, "v");

			service()->emit_return(
				service()->emit_isfinite(fn.arg(0)),
				service()->param_abi(false)
				);
		}
		else if( intr->unmangled_name() == "isnan" )
		{
			function_t& fn = service()->fn();
			assert(fn.arg_size() == 1);
			fn.arg_name(0, "v");

			service()->emit_return(
				service()->emit_isnan(fn.arg(0)),
				service()->param_abi(false)
				);
		}
		else if( intr->unmangled_name() == "min" )
		{
			function_t& fn = service()->fn();
			assert(fn.arg_size() == 2);

			fn.arg_name(0, "v0");
			fn.arg_name(1, "v1");

			value_t v0 = fn.arg(0);
			value_t v1 = fn.arg(1);

			value_t ret = service()->emit_select(service()->emit_cmp_lt(v0, v1), v0, v1);
			service()->emit_return( ret, service()->param_abi(false) );
		}
		else if( intr->unmangled_name() == "max" )
		{
			function_t& fn = service()->fn();
			assert(fn.arg_size() == 2);

			fn.arg_name(0, "v0");
			fn.arg_name(1, "v1");

			value_t v0 = fn.arg(0);
			value_t v1 = fn.arg(1);

			value_t ret = service()->emit_select(service()->emit_cmp_gt(v0, v1), v0, v1);
			service()->emit_return( ret, service()->param_abi(false) );
		}
		else if( intr->unmangled_name() == "frac" )
		{
			// frac = abs(v) - abs( floor(v) );
			function_t& fn = service()->fn();
			assert(fn.arg_size() == 1);

			fn.arg_name(0, "v");

			value_t v = fn.arg(0);
			value_t abs_value = service()->emit_abs(v);
			value_t floor_value = service()->emit_unary_ps( "sasl.floor.f32", abs_value );
			value_t ret = service()->emit_sub(abs_value, floor_value);
			service()->emit_return( ret, service()->param_abi(false) );
		}
		else if( intr->unmangled_name() == "saturate" )
		{
			function_t& fn = service()->fn();
			assert(fn.arg_size() == 1);

			fn.arg_name(0, "v");

			service()->emit_return( service()->emit_saturate( fn.arg(0) ), service()->param_abi(false) );
		}
		else if( intr->unmangled_name() == "rcp" )
		{
			function_t& fn = service()->fn();
			assert(fn.arg_size() == 1);

			fn.arg_name(0, "v");

			value_t v = fn.arg(0);
			value_t one_value = service()->one_value(v);

			value_t ret = service()->emit_div(one_value, v);
			
			service()->emit_return( ret, service()->param_abi(false) );
		}
		else if( intr->unmangled_name() == "normalize" )
		{
			function_t& fn = service()->fn();
			assert(fn.arg_size() == 1);

			fn.arg_name(0, "v");

			value_t v = fn.arg(0);
			value_t length_sqr = service()->emit_dot(v, v);
			value_t length = service()->emit_sqrt(length_sqr);
			value_t ret = service()->emit_div(v, length);

			service()->emit_return( ret, service()->param_abi(false) );
		}
		else if( intr->unmangled_name() == "reflect" )
		{
			function_t& fn = service()->fn();
			assert(fn.arg_size() == 2);

			fn.arg_name(0, "i");
			fn.arg_name(1, "n");

			value_t i = fn.arg(0);
			value_t n = fn.arg(1);

			value_t two = service()->create_constant_scalar( 2.0f, NULL, scalar_of( i.hint() ) );
			value_t double_dot = service()->emit_mul_comp( two, service()->emit_dot(i, n) );
			value_t ret = service()->emit_sub( i, service()->emit_mul_comp(double_dot, n) );

			service()->emit_return( ret, service()->param_abi(false) );
		}
		else if( intr->unmangled_name() == "sign" )
		{
			function_t& fn = service()->fn();
			assert(fn.arg_size() == 1);
			fn.arg_name(0, "v");
			service()->emit_return( service()->emit_sign( fn.arg(0) ), service()->param_abi(false) );
		}
		else if( intr->unmangled_name() == "smoothstep" )
		{
			function_t& fn = service()->fn();
			assert(fn.arg_size() == 3);

			fn.arg_name(0, "v0");
			fn.arg_name(1, "v1");
			fn.arg_name(2, "v2");

			value_t min_v = fn.arg(0);
			value_t max_v = fn.arg(1);
			value_t v     = fn.arg(2);

			// t = clamp((x - min) / (max-min), 0.0, 1.0);
			// return t * t * (3.0 - 2.0 * t);

			value_t t = service()->emit_div(
				service()->emit_sub(v, min_v),
				service()->emit_sub(max_v, min_v)
				);
			t = service()->emit_saturate(t);

			value_t two = service()->numeric_value(t, 2.0, 2);
			value_t three = service()->numeric_value(t, 3.0, 3);

			value_t ret = service()->emit_sub(
				three,
				service()->emit_mul_comp(two, t)
				);
			ret = service()->emit_mul_comp(ret, t);
			ret = service()->emit_mul_comp(ret, t);

			service()->emit_return( ret, service()->param_abi(false) );
		}
		else if( intr->unmangled_name() == "refract" )
		{
			function_t& fn = service()->fn();
			assert(fn.arg_size() == 3);

			fn.arg_name(0, "i");
			fn.arg_name(1, "n");
			fn.arg_name(2, "eta");

			// k = 1.0 - eta * eta * (1.0 - dot(N, I) * dot(N, I));
			// if (k < 0.0)
			//	 R = genType(0.0);       // or genDType(0.0)
			// else
			//	 R = eta * I - (eta * dot(N, I) + sqrt(k)) * N;

			value_t i	= fn.arg(0);
			value_t n	= fn.arg(1);
			value_t eta = fn.arg(2);

			value_t one			= service()->one_value(i);
			value_t zero		= service()->null_value(i.hint(), i.abi());
			value_t eta_x_eta	= service()->emit_mul_comp(eta, eta);
			value_t n_dot_i		= service()->emit_dot(n, i);
			value_t eta_x_i		= service()->emit_mul_comp(eta, i);

			value_t k, r;

			k = service()->emit_mul_comp(n_dot_i, n_dot_i);
			k = service()->emit_sub(one, k);
			k = service()->emit_mul_comp(eta_x_eta, k);
			k = service()->emit_sub(one, k);

			value_t flag = service()->emit_cmp_lt(k, zero);
			k = service()->emit_select(flag, zero, k);

			r = service()->emit_mul_comp(eta, n_dot_i);
			r = service()->emit_add( r, service()->emit_sqrt(k) );
			r = service()->emit_mul_comp(r, n);
			r = service()->emit_sub(eta_x_i, r);

			r = service()->emit_select(flag, zero, r);

			service()->emit_return( r, service()->param_abi(false) );
		}
		else if (intr->unmangled_name() == "step")
		{
			function_t& fn = service()->fn();
			assert(fn.arg_size() == 2);

			fn.arg_name(0, "y");
			fn.arg_name(1, "x");

			value_t y = fn.arg(0);
			value_t x = fn.arg(1);

			value_t ret = service()->emit_select(
				service()->emit_cmp_ge(x, y),
				service()->one_value(x),
				service()->null_value( x.hint(), x.abi() ) );

			service()->emit_return( ret, service()->param_abi(false) );
		}
		else if (intr->unmangled_name() == "mad")
		{
			function_t& fn = service()->fn();
			assert(fn.arg_size() == 3);

			fn.arg_name(0, "m");
			fn.arg_name(1, "a");
			fn.arg_name(2, "b");

			value_t m = fn.arg(0);
			value_t a = fn.arg(1);
			value_t b = fn.arg(2);

			value_t ret = service()->emit_add(service()->emit_mul_comp(m, a), b);
			service()->emit_return( ret, service()->param_abi(false) );
		}
		else if( intr->unmangled_name() == "faceforward" )
		{
			function_t& fn = service()->fn();
			assert(fn.arg_size() == 3);

			fn.arg_name(0, "n");
			fn.arg_name(1, "i");
			fn.arg_name(2, "ng");

			// -n * sign( dot(i,ng) )
			value_t n  = fn.arg(0);
			value_t i  = fn.arg(1);
			value_t ng = fn.arg(2);

			value_t zero = service()->null_value( n.hint(), n.abi() );
			value_t i_dot_ng = service()->emit_dot(i, ng);
			value_t ret = service()->emit_select(
				service()->emit_cmp_lt( i_dot_ng, service()->null_value( i_dot_ng.hint(), i_dot_ng.abi() ) ),
				n, service()->emit_sub(zero, n)
				);
			service()->emit_return( ret, service()->param_abi(false) );
		}
		else if (intr->unmangled_name() == "lit")
		{
			function_t& fn = service()->fn();
			assert(fn.arg_size() == 3);

			fn.arg_name(0, "n_dot_l");
			fn.arg_name(1, "n_dot_h");
			fn.arg_name(2, "m");

			// ambient = 1.
			// diffuse = (n_dot_l < 0) ? 0 : n_dot_l.
			// specular = (n_dot_l < 0) || (n_dot_h < 0) ? 0 : (n_dot_h* m).
			value_t n_dot_l = fn.arg(0);
			value_t n_dot_h = fn.arg(1);
			value_t m		= fn.arg(2);

			value_t one = service()->one_value(n_dot_l);
			value_t zero = service()->null_value( n_dot_l.hint(), n_dot_l.abi() );
			value_t ambient = one;
			value_t diffuse = service()->emit_select(
				service()->emit_cmp_lt(n_dot_l, zero), zero, n_dot_l );
			value_t specular = service()->emit_select(
				service()->emit_or(
					service()->emit_cmp_lt(n_dot_l, zero),
					service()->emit_cmp_lt(n_dot_h, zero)
					), zero, service()->emit_mul_comp(n_dot_h, m) );

			vector<value_t> values;
			values.push_back(one);
			values.push_back(diffuse);
			values.push_back(specular);
			values.push_back(one);
			abis promoted_abi = service()->promote_abi(one.abi(), diffuse.abi(), specular.abi() );
			value_t lit_packed = service()->create_vector( values, promoted_abi );

			service()->emit_return( lit_packed, service()->param_abi(false) );
		}
		else
		{
			EFLIB_ASSERT( !"Unprocessed intrinsic.", intr->unmangled_name().c_str() );
		}
		service()->clean_empty_blocks();
	}
}

END_NS_SASL_CODE_GENERATOR();
