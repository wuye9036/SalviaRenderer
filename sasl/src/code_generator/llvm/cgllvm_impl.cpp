#include <sasl/include/code_generator/llvm/cgllvm_impl.h>

#include <sasl/enums/enums_helper.h>
#include <sasl/include/code_generator/llvm/cgllvm_contexts.h>
#include <sasl/include/code_generator/llvm/cgllvm_globalctxt.h>
#include <sasl/include/code_generator/llvm/cgllvm_type_converters.h>
#include <sasl/include/semantic/name_mangler.h>
#include <sasl/include/semantic/semantic_infos.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/semantic/type_checker.h>
#include <sasl/include/semantic/type_converter.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/expression.h>
#include <sasl/include/syntax_tree/statement.h>
#include <sasl/include/syntax_tree/program.h>

#include <eflib/include/diagnostics/assert.h>
#include <eflib/include/metaprog/util.h>
#include <eflib/include/platform/disable_warnings.h>
#include <boost/assign/std/vector.hpp>
#include <eflib/include/platform/enable_warnings.h>
#include <string>

BEGIN_NS_SASL_CODE_GENERATOR();


using namespace syntax_tree;
using namespace boost::assign;
using namespace llvm;

using semantic::const_value_si;
using semantic::extract_semantic_info;
using semantic::symbol;
using semantic::type_equal;
using semantic::operator_name;
using semantic::type_info_si;

using boost::shared_ptr;
using boost::any;
using boost::any_cast;

using std::vector;

typedef shared_ptr<cgllvm_common_context> common_ctxt_handle;

#define is_node_class( handle_of_node, typecode ) ( (handle_of_node)->node_class() == syntax_node_types::typecode )

cgllvm_common_context const * any_to_cgctxt_ptr( const any& any_val  ){
	return any_cast<cgllvm_common_context>(&any_val);
}

cgllvm_common_context* any_to_cgctxt_ptr( any& any_val ){
	return any_cast<cgllvm_common_context>(&any_val);
}

#define data_as_cgctxt_ptr() ( any_to_cgctxt_ptr(*data) )

//////////////////////////////////////////////////////////////////////////
// utility functions.

template< typename NodeT >
static common_ctxt_handle extract_common_ctxt( NodeT& v ){
	return extract_codegen_context<cgllvm_common_context>(v);
}
template< typename NodeT >
static common_ctxt_handle extract_common_ctxt( boost::shared_ptr<NodeT> v ){
	return extract_codegen_context<cgllvm_common_context>(v);
}
template< typename NodeT >
static common_ctxt_handle get_common_ctxt( NodeT& v ){
	return get_or_create_codegen_context<cgllvm_common_context>(v);
}
template< typename NodeT >
static common_ctxt_handle get_common_ctxt( boost::shared_ptr<NodeT> v ){
	return get_or_create_codegen_context<cgllvm_common_context>(v);
}

template<typename NodeT>
static common_ctxt_handle parent_ctxt( boost::shared_ptr<NodeT> v ){
	boost::shared_ptr<symbol> parent_sym = v->symbol()->parent();
	if( parent_sym ){
		return extract_codegen_context<cgllvm_common_context>( parent_sym->node() );
	}
	return common_ctxt_handle();
}

//////////////////////////////////////////////////////////////////////////
//
#define SASL_VISITOR_TYPE_NAME llvm_code_generator

llvm_code_generator::llvm_code_generator( )
{
}

template <typename NodeT> any& llvm_code_generator::visit_child( any& child_ctxt, const any& init_data, shared_ptr<NodeT> child )
{
	child_ctxt = init_data;
	EFLIB_ASSERT( !child_ctxt.empty(), "" );
	return visit_child( child_ctxt, child );
}

template <typename NodeT> any& llvm_code_generator::visit_child( any& child_ctxt, shared_ptr<NodeT> child )
{
	child->accept( this, &child_ctxt );
	return child_ctxt;
}

SASL_VISIT_NOIMPL( unary_expression );
SASL_VISIT_NOIMPL( cast_expression );

SASL_VISIT_DEF( binary_expression ){
	// generate left and right expr.
	v.left_expr->accept( this, data );
	v.right_expr->accept( this, data );

	boost::shared_ptr<type_specifier> ltype = extract_semantic_info<type_info_si>(v.left_expr)->type_info();
	boost::shared_ptr<type_specifier> rtype = extract_semantic_info<type_info_si>(v.right_expr)->type_info();

	EFLIB_ASSERT_AND_IF(
		( is_node_class(ltype, buildin_type ) && is_node_class(rtype, buildin_type ) ),
		"Operators of buildin type are supported only." )
	{
		return;
	}

	//////////////////////////////////////////////////////////////////////////
	// type conversation for matching the operator prototype

	// get an overloadable prototype.
	std::vector< boost::shared_ptr<expression> > args;
	args += v.left_expr, v.right_expr;

	symbol::overloads_t overloads
		= data_as_cgctxt_ptr()->sym.lock()->find_overloads( operator_name( v.op ), typeconv, args );

	EFLIB_ASSERT_AND_IF( !overloads.empty(), "Error report: no prototype could match the expression." ){
		return;
	}
	EFLIB_ASSERT_AND_IF( overloads.size() == 1, "Error report: prototype was ambigous." ){
		return;
	}

	boost::shared_ptr<function_type> op_proto = overloads[0]->node()->typed_handle<function_type>();

	// convert value type to match proto type.
	if( ! type_equal( op_proto->params[0]->param_type, ltype ) ){
		typeconv->convert( op_proto->params[0]->param_type, v.left_expr );
	}
	if( ! type_equal( op_proto->params[0]->param_type, rtype ) ){
		typeconv->convert( op_proto->params[0]->param_type, v.right_expr );
	}

	// use type-converted value to generate code.
	Value* lval = extract_common_ctxt( v.left_expr )->val;
	Value* rval = extract_common_ctxt( v.right_expr )->val;

	if (v.op == operators::add){
		get_common_ctxt(v)->val = ctxt->builder()->CreateAdd( lval, rval, "" );
	} else if ( v.op == operators::sub ){
		get_common_ctxt(v)->val = ctxt->builder()->CreateSub( lval, rval, "" );
	} else if ( v.op == operators::mul ){
		get_common_ctxt(v)->val = ctxt->builder()->CreateMul( lval, rval, "" );
	} else if ( v.op == operators::div ){
		EFLIB_INTERRUPT( "Division is not supported yet." );
	}
}

SASL_VISIT_NOIMPL( expression_list );
SASL_VISIT_NOIMPL( cond_expression );
SASL_VISIT_NOIMPL( index_expression );
SASL_VISIT_NOIMPL( call_expression );
SASL_VISIT_NOIMPL( member_expression );

SASL_VISIT_DEF( constant_expression ){
//	SASL_REWRITE_DATA_AS_SYMBOL();

	boost::shared_ptr<const_value_si> c_si = extract_semantic_info<const_value_si>(v);
	c_si->type_info()->accept( this, data );

	if( c_si->value_type() == buildin_type_code::_sint32 ){
		get_common_ctxt(v)->val = ConstantInt::get( extract_common_ctxt( c_si->type_info() )->type, uint64_t( c_si->value<int32_t>() ), true );
	} else if ( c_si->value_type() == buildin_type_code::_uint32 ) {
		get_common_ctxt(v)->val = ConstantInt::get( extract_common_ctxt( c_si->type_info() )->type, uint64_t( c_si->value<uint32_t>() ), true );
	} else {
		EFLIB_ASSERT_UNIMPLEMENTED();
	}
}

SASL_VISIT_NOIMPL( identifier );
SASL_VISIT_DEF( variable_expression )
{
	UNREF_PARAM( v );
	UNREF_PARAM( data );

	// do nothing
}

// declaration & type specifier
SASL_VISIT_NOIMPL( initializer );
SASL_VISIT_NOIMPL( expression_initializer );
SASL_VISIT_NOIMPL( member_initializer );
SASL_VISIT_NOIMPL( declaration );
SASL_VISIT_NOIMPL( variable_declaration );

SASL_VISIT_NOIMPL( type_definition );
SASL_VISIT_NOIMPL( type_specifier );
SASL_VISIT_DEF( buildin_type ){
	EFLIB_ASSERT_AND_IF( !data_as_cgctxt_ptr()->sym.expired(), "Null symbol." ){
		return;
	}

	if ( v.codegen_ctxt() ){
		*data = *( v.codegen_ctxt() );
		return;
	}
	
	const Type* ret_type = NULL;
	bool sign = false;

	if ( sasl_ehelper::is_void( v.value_typecode ) ){
		ret_type = Type::getVoidTy( ctxt->context() );
	} else if( sasl_ehelper::is_scalar(v.value_typecode) ){
		if( sasl_ehelper::is_integer(v.value_typecode) ){
			ret_type = IntegerType::get( ctxt->context(), (unsigned int)sasl_ehelper::storage_size( v.value_typecode ) << 3 );
			sign = sasl_ehelper::is_signed( v.value_typecode );
		} else if ( v.value_typecode == buildin_type_code::_float ){
			ret_type = Type::getFloatTy( ctxt->context() );
		} else if ( v.value_typecode == buildin_type_code::_double ){
			ret_type = Type::getDoubleTy( ctxt->context() );
		}
	}

	std::string tips = v.value_typecode.name() + std::string(" was not supported yet.");
	EFLIB_ASSERT_AND_IF( ret_type, tips.c_str() ){
		return;
	}

	data_as_cgctxt_ptr()->type = ret_type;
	data_as_cgctxt_ptr()->is_signed = sign;

	*get_common_ctxt(v) = *(data_as_cgctxt_ptr());
}

SASL_VISIT_NOIMPL( array_type );
SASL_VISIT_NOIMPL( struct_type );
SASL_VISIT_DEF( parameter ){

	data_as_cgctxt_ptr()->sym = v.symbol();

	any child_ctxt_init = *data;
	any child_ctxt;

	visit_child( child_ctxt, child_ctxt_init, v.param_type );
	data_as_cgctxt_ptr()->type = any_to_cgctxt_ptr(child_ctxt)->type;
	data_as_cgctxt_ptr()->is_signed = any_to_cgctxt_ptr(child_ctxt)->is_signed;
	if (v.init){
		visit_child( child_ctxt, child_ctxt_init, v.init );
	}

	*get_common_ctxt(v) = *(data_as_cgctxt_ptr());

}

SASL_VISIT_DEF( function_type ){
	
	data_as_cgctxt_ptr()->sym = v.symbol();

	any child_ctxt_init = *data;
	any child_ctxt;

	// Generate return types.
	visit_child( child_ctxt, child_ctxt_init, v.retval_type );
	const llvm::Type* ret_type = any_to_cgctxt_ptr(child_ctxt)->type;

	EFLIB_ASSERT_AND_IF( ret_type, "ret_type" ){
		return;
	}

	// Generate paramenter types.
	vector< const llvm::Type*> param_types;
	for( vector< boost::shared_ptr<parameter> >::iterator it = v.params.begin(); it != v.params.end(); ++it ){
		visit_child( child_ctxt, child_ctxt_init, v.retval_type );
		if ( any_to_cgctxt_ptr(child_ctxt)->type ){
			param_types.push_back( any_to_cgctxt_ptr(child_ctxt)->type );
		} else {
			EFLIB_ASSERT_AND_IF( ret_type, "Error occurs while parameter parsing." ){
				return;
			}
		}
	}

	// Create function.
	llvm::FunctionType* ftype = llvm::FunctionType::get( ret_type, param_types, false );
	data_as_cgctxt_ptr()->func_type = ftype;
	llvm::Function* fn = 
		Function::Create( ftype, Function::ExternalLinkage, v.name->str, ctxt->module() );
	data_as_cgctxt_ptr()->func = fn;
	any_to_cgctxt_ptr(child_ctxt_init)->parent_func = fn;

	// Register parameter names.
	llvm::Function::arg_iterator arg_it = fn->arg_begin();
	for( size_t arg_idx = 0; arg_idx < fn->arg_size(); ++arg_idx, ++arg_it){
		boost::shared_ptr<parameter> par = v.params[arg_idx];
		arg_it->setName( par->symbol()->unmangled_name() );
		common_ctxt_handle par_ctxt = extract_common_ctxt( par->param_type );
		par_ctxt->arg = boost::addressof( *arg_it );
	}


	// Create function body.
	if ( v.body ){
		visit_child( child_ctxt, child_ctxt_init, v.body );
	}

	*get_common_ctxt(v) = *( data_as_cgctxt_ptr() );
}

// statement
SASL_VISIT_NOIMPL( statement );
SASL_VISIT_NOIMPL( declaration_statement );
SASL_VISIT_NOIMPL( if_statement );
SASL_VISIT_NOIMPL( while_statement );
SASL_VISIT_NOIMPL( dowhile_statement );
SASL_VISIT_NOIMPL( case_label );
SASL_VISIT_NOIMPL( switch_statement );

SASL_VISIT_DEF( compound_statement ){
	data_as_cgctxt_ptr()->sym = v.symbol();

	any child_ctxt_init = *data;
	any child_ctxt;
	BasicBlock* bb = BasicBlock::Create(
		ctxt->context(),
		v.symbol()->mangled_name(),
		data_as_cgctxt_ptr()->parent_func
		);

	data_as_cgctxt_ptr()->block = bb;

	ctxt->builder()->SetInsertPoint(bb);
	for ( std::vector< boost::shared_ptr<statement> >::iterator it = v.stmts.begin();
		it != v.stmts.end(); ++it)
	{
		visit_child( child_ctxt, child_ctxt_init, *it );
	}

	*get_common_ctxt(v) = *( data_as_cgctxt_ptr() );
}

SASL_VISIT_DEF( expression_statement ){
//	SASL_REWRITE_DATA_AS_SYMBOL();
	v.expr->accept( this, data );
}

SASL_VISIT_DEF( jump_statement ){

	if (v.jump_expr){
		v.jump_expr->accept( this, data );
	}
	ReturnInst* ret_ins = NULL;
	if ( v.code == jump_mode::_return ){
		if ( !v.jump_expr ){
			ret_ins = ctxt->builder()->CreateRetVoid();
		} else {
			ret_ins = ctxt->builder()->CreateRet( extract_common_ctxt(v.jump_expr)->val );
		}
	}
	get_common_ctxt(v)->ret_ins = ret_ins;
}

SASL_VISIT_NOIMPL( ident_label );

SASL_VISIT_DEF( program ){
	UNREF_PARAM( data );
	if ( ctxt ){
		return;
	}

	ctxt = create_codegen_context<cgllvm_global_context>(v.handle());
	ctxt->create_module( v.name );
	typeconv = create_type_converter( ctxt->builder() );

	any child_ctxt = cgllvm_common_context();

	for( vector< boost::shared_ptr<declaration> >::iterator
		it = v.decls.begin(); it != v.decls.end(); ++it )
	{
		visit_child( child_ctxt, (*it) );
	}
}

SASL_VISIT_NOIMPL( for_statement );

boost::shared_ptr<llvm_code> llvm_code_generator::generated_module(){
	return boost::shared_polymorphic_cast<llvm_code>(ctxt);
}

END_NS_SASL_CODE_GENERATOR();
