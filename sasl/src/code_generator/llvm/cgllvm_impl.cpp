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
#include <boost/assign/std/vector.hpp>
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

void llvm_code_generator::visit( unary_expression& ){
	
}

void llvm_code_generator::visit( cast_expression& ){}
void llvm_code_generator::visit( binary_expression& v){
	//// generate left and right expr.
	v.left_expr->accept(this);
	v.right_expr->accept(this);

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
		= v.symbol()->find_overloads( operator_name( v.op ), typeconv, args );
	
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
void llvm_code_generator::visit( expression_list& ){}
void llvm_code_generator::visit( cond_expression& ){}
void llvm_code_generator::visit( index_expression& ){}
void llvm_code_generator::visit( call_expression& ){}
void llvm_code_generator::visit( member_expression& ){}

void llvm_code_generator::visit( constant_expression& v ){
	boost::shared_ptr<const_value_si> c_si = extract_semantic_info<const_value_si>(v);
	c_si->type_info()->accept( this );

	if( c_si->value_type() == buildin_type_code::_sint32 ){
		get_common_ctxt(v)->val = ConstantInt::get( extract_common_ctxt( c_si->type_info() )->type, uint64_t( c_si->value<int32_t>() ), true );
	} else {
		EFLIB_ASSERT_UNIMPLEMENTED();
	}
}

void llvm_code_generator::visit( identifier& ){

}
void llvm_code_generator::visit( variable_expression& ){

}
// declaration & type specifier
void llvm_code_generator::visit( initializer& ){}
void llvm_code_generator::visit( expression_initializer& ){}
void llvm_code_generator::visit( member_initializer& ){}
void llvm_code_generator::visit( declaration& ){}
void llvm_code_generator::visit( variable_declaration& ){
}

void llvm_code_generator::visit( type_definition& ){}
void llvm_code_generator::visit( type_specifier& ){
}
void llvm_code_generator::visit( buildin_type& v ){
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
void llvm_code_generator::visit( array_type& ){}
void llvm_code_generator::visit( struct_type& ){}
void llvm_code_generator::visit( parameter& v ){
	v.param_type->accept( this );
	if (v.init){
		v.init->accept( this );
	}
	get_common_ctxt(v)->type = get_common_ctxt(v.param_type)->type;
	get_common_ctxt(v)->is_signed = get_common_ctxt(v.param_type)->is_signed;
}

void llvm_code_generator::visit( function_type& v ){

	// skip if context existed.
	if ( v.codegen_ctxt() ) { return; }
	common_ctxt_handle fctxt = get_common_ctxt( v );

	// Generate return types.
	v.retval_type->accept( this );
	const llvm::Type* ret_type = extract_common_ctxt(v.retval_type)->type;
	
	EFLIB_ASSERT_AND_IF( ret_type, "ret_type" ){
		return;
	}

	// Generate paramenter types.
	vector< const llvm::Type*> param_types;
	for( vector< boost::shared_ptr<parameter> >::iterator it = v.params.begin(); it != v.params.end(); ++it ){
		(*it)->accept(this);
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
		v.body->accept( this );
	}
}

// statement
void llvm_code_generator::visit( statement& ){
}
void llvm_code_generator::visit( declaration_statement& ){}
void llvm_code_generator::visit( if_statement& ){}
void llvm_code_generator::visit( while_statement& ){}
void llvm_code_generator::visit( dowhile_statement& ){}
void llvm_code_generator::visit( case_label& ){}
void llvm_code_generator::visit( switch_statement& ){}

void llvm_code_generator::visit( compound_statement& v ){
	
	BasicBlock* bb = BasicBlock::Create(
		ctxt->context(),
		v.symbol()->mangled_name(),
		extract_common_ctxt( v.symbol()->parent()->node() )->func 
		);
	get_common_ctxt(v.handle())->block = bb;

	ctxt->builder()->SetInsertPoint(bb);
	for ( std::vector< boost::shared_ptr<statement> >::iterator it = v.stmts.begin();
		it != v.stmts.end(); ++it){
		(*it)->accept( this );
	}
}

void llvm_code_generator::visit( expression_statement& v ){
	v.expr->accept( this );
}

void llvm_code_generator::visit( jump_statement& v ){
	if (v.jump_expr){
		v.jump_expr->accept( this );
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

void llvm_code_generator::visit( ident_label& ){ }

void llvm_code_generator::visit( program& v ){
	if ( ctxt ){
		return;
	}

	ctxt = create_codegen_context<cgllvm_global_context>(v.handle());
	ctxt->create_module( v.name );
	typeconv = create_type_converter( ctxt->builder() );

	for( vector< boost::shared_ptr<declaration> >::iterator
		it = v.decls.begin(); it != v.decls.end(); ++it ){
		(*it)->accept( this );
	}
}

void llvm_code_generator::visit( sasl::syntax_tree::for_statement& /*v*/ ){
}

boost::shared_ptr<llvm_code> llvm_code_generator::generated_module(){
	return boost::shared_polymorphic_cast<llvm_code>(ctxt);
}

END_NS_SASL_CODE_GENERATOR();