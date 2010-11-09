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

using namespace std;
using namespace syntax_tree;
using namespace boost::assign;
using namespace llvm;

using semantic::const_value_si;
using semantic::extract_semantic_info;
using semantic::symbol;
using semantic::type_equal;
using semantic::operator_name;
using semantic::type_info_si;

typedef boost::shared_ptr<cgllvm_common_context> common_ctxt_handle;

#define is_node_class( handle_of_node, typecode ) ( (handle_of_node)->node_class() == syntax_node_types::##typecode )

#define SASL_REWRITE_DATA_AS_SYMBOL()	\
	::boost::any sym_data( v.symbol() );	\
	data = v.symbol() ? &sym_data : data;

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
llvm_code_generator::llvm_code_generator( )
{
}

#define SASL_VISITOR_TYPE_NAME llvm_code_generator

SASL_VISIT_NOIMPL( unary_expression );
SASL_VISIT_NOIMPL( cast_expression );

SASL_VISIT_DEF( binary_expression ){
	SASL_REWRITE_DATA_AS_SYMBOL();

	//// generate left and right expr.
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
		= ::boost::any_cast< ::boost::shared_ptr<symbol> >( *data )->find_overloads( operator_name( v.op ), typeconv, args );
	
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
	SASL_REWRITE_DATA_AS_SYMBOL();

	boost::shared_ptr<const_value_si> c_si = extract_semantic_info<const_value_si>(v);
	c_si->type_info()->accept( this, data );

	if( c_si->value_type() == buildin_type_code::_sint32 ){
		get_common_ctxt(v)->val = ConstantInt::get( extract_common_ctxt( c_si->type_info() )->type, uint64_t( c_si->value<int32_t>() ), true );
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
	SASL_REWRITE_DATA_AS_SYMBOL();

	if ( v.codegen_ctxt() ){ return; }
	common_ctxt_handle type_ctxt = get_common_ctxt(v);
	if ( sasl_ehelper::is_void( v.value_typecode ) ){
		type_ctxt->type = Type::getVoidTy( ctxt->context() );
	} else if( sasl_ehelper::is_scalar(v.value_typecode) ){
		if( sasl_ehelper::is_integer(v.value_typecode) ){
			type_ctxt->type = IntegerType::get( ctxt->context(), (unsigned int)sasl_ehelper::storage_size( v.value_typecode ) << 3 );
			type_ctxt->is_signed = sasl_ehelper::is_signed( v.value_typecode );
		} else if ( v.value_typecode == buildin_type_code::_float ){
			type_ctxt->type = Type::getFloatTy( ctxt->context() );
		} else if ( v.value_typecode == buildin_type_code::_double ){
			type_ctxt->type = Type::getDoubleTy( ctxt->context() );
		}
	}

	std::string tips = v.value_typecode.name() + std::string(" was not supported yet.");
	EFLIB_ASSERT( type_ctxt->type, tips.c_str() );
}
SASL_VISIT_NOIMPL( array_type );
SASL_VISIT_NOIMPL( struct_type );
SASL_VISIT_DEF( parameter ){

	SASL_REWRITE_DATA_AS_SYMBOL();
	
	v.param_type->accept( this, data );
	if (v.init){
		v.init->accept( this, data );
	}
	get_common_ctxt(v)->type = get_common_ctxt(v.param_type)->type;
	get_common_ctxt(v)->is_signed = get_common_ctxt(v.param_type)->is_signed;
}

SASL_VISIT_DEF( function_type ){
	SASL_REWRITE_DATA_AS_SYMBOL();

	// skip if context existed.
	if ( v.codegen_ctxt() ) { return; }
	common_ctxt_handle fctxt = get_common_ctxt( v );

	// Generate return types.
	v.retval_type->accept( this, data );
	const llvm::Type* ret_type = extract_common_ctxt(v.retval_type)->type;
	
	EFLIB_ASSERT_AND_IF( ret_type, "ret_type" ){
		return;
	}

	// Generate paramenter types.
	vector< const llvm::Type*> param_types;
	for( vector< boost::shared_ptr<parameter> >::iterator it = v.params.begin(); it != v.params.end(); ++it ){
		(*it)->accept( this, data );
		common_ctxt_handle par_ctxt = get_common_ctxt( (*it) );
		if ( par_ctxt->type ){
			param_types.push_back( par_ctxt->type );
		} else {
			EFLIB_ASSERT_AND_IF( ret_type, "Error occurs while parameter parsing." ){
				return;
			}
		}
	}

	// Create function.
	fctxt->func_type = llvm::FunctionType::get( ret_type, param_types, false );
	fctxt->func = Function::Create( fctxt->func_type, Function::ExternalLinkage, v.name->str, ctxt->module() );

	// Register parameter names.
	llvm::Function::arg_iterator arg_it = fctxt->func->arg_begin();
	for( size_t arg_idx = 0; arg_idx < fctxt->func->arg_size(); ++arg_idx, ++arg_it){
		boost::shared_ptr<parameter> par = v.params[arg_idx];
		arg_it->setName( par->symbol()->unmangled_name() );
		common_ctxt_handle par_ctxt = get_common_ctxt( par->param_type );
		par_ctxt->arg = boost::addressof( *arg_it );
	}

	// Create function body.
	if ( v.body ){
		v.body->accept( this, data );
	}
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
	
	SASL_REWRITE_DATA_AS_SYMBOL();

	BasicBlock* bb = BasicBlock::Create(
		ctxt->context(),
		v.symbol()->mangled_name(),
		extract_common_ctxt( v.symbol()->parent()->node() )->func 
		);
	get_common_ctxt(v.handle())->block = bb;

	ctxt->builder()->SetInsertPoint(bb);
	for ( std::vector< boost::shared_ptr<statement> >::iterator it = v.stmts.begin();
		it != v.stmts.end(); ++it){
		(*it)->accept( this, data );
	}
}

SASL_VISIT_DEF( expression_statement ){
	SASL_REWRITE_DATA_AS_SYMBOL();
	v.expr->accept( this, data );
}

SASL_VISIT_DEF( jump_statement ){

	SASL_REWRITE_DATA_AS_SYMBOL();

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

	::boost::any sym_data( v.symbol() );

	for( vector< boost::shared_ptr<declaration> >::iterator
		it = v.decls.begin(); it != v.decls.end(); ++it ){
		(*it)->accept( this, &sym_data );
	}
}

SASL_VISIT_NOIMPL( for_statement );

boost::shared_ptr<llvm_code> llvm_code_generator::generated_module(){
	return boost::shared_polymorphic_cast<llvm_code>(ctxt);
}

END_NS_SASL_CODE_GENERATOR();