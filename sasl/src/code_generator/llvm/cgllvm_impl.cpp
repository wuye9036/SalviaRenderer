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
#include <eflib/include/platform/boost_begin.h>
#include <boost/assign/std/vector.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <eflib/include/platform/boost_end.h>

#include <string>

BEGIN_NS_SASL_CODE_GENERATOR();

using namespace syntax_tree;
using namespace boost::assign;
using namespace llvm;

using semantic::const_value_si;
using semantic::extract_semantic_info;
using semantic::global_si;
using semantic::operator_name;
using semantic::statement_si;
using semantic::symbol;
using semantic::type_converter;
using semantic::type_entry;
using semantic::type_equal;
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

// Process assign
void llvm_code_generator::do_assign( any* data, shared_ptr<expression> lexpr, shared_ptr<expression> rexpr )
{
	shared_ptr<type_info_si> larg_tsi = extract_semantic_info<type_info_si>(lexpr);
	shared_ptr<type_info_si> rarg_tsi = extract_semantic_info<type_info_si>(rexpr);

	if ( larg_tsi->entry_id() != rarg_tsi->entry_id() ){
		if( typeconv->implicit_convertible( larg_tsi->entry_id(), rarg_tsi->entry_id() ) ){
			typeconv->convert( larg_tsi->type_info(), rexpr );
		} else {
			assert( !"Expression could not converted to storage type." );
		}
	}

	Value* addr = extract_common_ctxt(lexpr)->addr;
	Value* val = extract_common_ctxt(rexpr)->val;

	ctxt->builder()->CreateStore( val, addr );

	data_as_cgctxt_ptr()->type = extract_common_ctxt(lexpr)->type;
	data_as_cgctxt_ptr()->addr = addr;
	data_as_cgctxt_ptr()->val = val;
}

Constant* llvm_code_generator::get_zero_filled_constant( boost::shared_ptr<type_specifier> typespec )
{
	if( typespec->node_class() == syntax_node_types::buildin_type ){
		buildin_type_code btc = typespec->value_typecode;
		if( sasl_ehelper::is_integer( btc ) ){
			return ConstantInt::get( extract_common_ctxt(typespec)->type, 0, sasl_ehelper::is_signed(btc) );
		}
		if( sasl_ehelper::is_real( btc ) ){
			return ConstantFP::get( extract_common_ctxt(typespec)->type, 0.0 );
		}
	}

	EFLIB_ASSERT_UNIMPLEMENTED();
	return NULL;
}

void llvm_code_generator::restart_block( boost::any* data ){
	BasicBlock* restart = BasicBlock::Create( ctxt->context(), "", data_as_cgctxt_ptr()->parent_func );
	ctxt->builder()->SetInsertPoint(restart);
}

SASL_VISIT_DEF_UNIMPL( unary_expression );
SASL_VISIT_DEF( cast_expression ){
	any child_ctxt_init = *data;
	any child_ctxt;

	visit_child( child_ctxt, child_ctxt_init, v.casted_type );
	visit_child( child_ctxt, child_ctxt_init, v.expr );

	shared_ptr<type_info_si> src_tsi = extract_semantic_info<type_info_si>( v.expr );
	shared_ptr<type_info_si> casted_tsi = extract_semantic_info<type_info_si>( v.casted_type );

	if( src_tsi->entry_id() != casted_tsi->entry_id() ){
		if( typeconv->convertible( casted_tsi->entry_id(), src_tsi->entry_id() ) == type_converter::cannot_conv ){
			// Here is code error. Compiler should report it.
			EFLIB_ASSERT_UNIMPLEMENTED();
		}
		get_common_ctxt(v)->type = extract_common_ctxt(v.casted_type)->type;
		typeconv->convert( v.handle(), v.expr );
	}

	data_as_cgctxt_ptr()->type = get_common_ctxt(v)->type;
	data_as_cgctxt_ptr()->val = get_common_ctxt(v)->val;
}

SASL_VISIT_DEF( binary_expression ){
	any child_ctxt_init = *data;
	any child_ctxt;

	visit_child( child_ctxt, child_ctxt_init, v.left_expr );
	visit_child( child_ctxt, child_ctxt_init, v.right_expr );

	if( v.op == operators::assign ){
		do_assign( data, v.left_expr, v.right_expr );
	} else {
		shared_ptr<type_info_si> larg_tsi = extract_semantic_info<type_info_si>(v.left_expr);
		shared_ptr<type_info_si> rarg_tsi = extract_semantic_info<type_info_si>(v.right_expr);

		//////////////////////////////////////////////////////////////////////////
		// type conversation for matching the operator prototype

		// get an overloadable prototype.
		std::vector< shared_ptr<expression> > args;
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

		shared_ptr<type_info_si> p0_tsi = extract_semantic_info<type_info_si>( op_proto->params[0] );
		shared_ptr<type_info_si> p1_tsi = extract_semantic_info<type_info_si>( op_proto->params[1] );

		// convert value type to match proto type.
		if( p0_tsi->entry_id() != larg_tsi->entry_id() ){
			if( !p0_tsi->type_info()->codegen_ctxt() ){
				visit_child( child_ctxt, child_ctxt_init,  op_proto->params[0]->param_type );
			}
			extract_common_ctxt(v.left_expr)->type = extract_common_ctxt( p0_tsi->type_info() )->type;
			typeconv->convert( p0_tsi->type_info(), v.left_expr );
		}
		if( p1_tsi->entry_id() != rarg_tsi->entry_id() ){
			if( !p1_tsi->type_info()->codegen_ctxt() ){
				visit_child( child_ctxt, child_ctxt_init, op_proto->params[1]->param_type );
			}
			extract_common_ctxt(v.right_expr)->type = extract_common_ctxt( p1_tsi->type_info() )->type;
			typeconv->convert( p1_tsi->type_info(), v.right_expr );
		}

		// use type-converted value to generate code.
		Value* lval = extract_common_ctxt( v.left_expr )->val;
		Value* rval = extract_common_ctxt( v.right_expr )->val;

		Value* retval = NULL;
		if( lval && rval ){

			buildin_type_code lbtc = p0_tsi->type_info()->value_typecode;
			buildin_type_code rbtc = p1_tsi->type_info()->value_typecode;

			if (v.op == operators::add){
				if( sasl_ehelper::is_real(lbtc) ){
					retval = ctxt->builder()->CreateFAdd( lval, rval, "" );
				} else if( sasl_ehelper::is_integer(lbtc) ){
					retval = ctxt->builder()->CreateAdd( lval, rval, "" );
				}
			} else if ( v.op == operators::sub ){
				retval = ctxt->builder()->CreateSub( lval, rval, "" );
			} else if ( v.op == operators::mul ){
				retval = ctxt->builder()->CreateMul( lval, rval, "" );
			} else if ( v.op == operators::div ){
				EFLIB_INTERRUPT( "Division is not supported yet." );
			} else if ( v.op == operators::less ){
				if(sasl_ehelper::is_real(lbtc)){
					retval = ctxt->builder()->CreateFCmpULT( lval, rval );
				} else if ( sasl_ehelper::is_integer(lbtc) ){
					if( sasl_ehelper::is_signed(lbtc) ){
						retval = ctxt->builder()->CreateICmpSLT(lval, rval);
					}
					if( sasl_ehelper::is_unsigned(lbtc) ){
						retval = ctxt->builder()->CreateICmpULT(lval, rval);
					}
				}
			} else {
				EFLIB_INTERRUPT( (boost::format("Operator %s is not supported yet.") % v.op.name() ).str().c_str() );
			}
		}

		data_as_cgctxt_ptr()->val = retval;
	}

	*get_common_ctxt(v) = *data_as_cgctxt_ptr();
}

SASL_VISIT_DEF_UNIMPL( expression_list );
SASL_VISIT_DEF_UNIMPL( cond_expression );
SASL_VISIT_DEF_UNIMPL( index_expression );
SASL_VISIT_DEF_UNIMPL( call_expression );
SASL_VISIT_DEF_UNIMPL( member_expression );

SASL_VISIT_DEF( constant_expression ){

	any child_ctxt_init = *data;
	any child_ctxt;

	boost::shared_ptr<const_value_si> c_si = extract_semantic_info<const_value_si>(v);
	if( !c_si->type_info()->codegen_ctxt() ){
		visit_child( child_ctxt, child_ctxt_init, c_si->type_info() );
	}

	Value* retval = NULL;
	if( c_si->value_type() == buildin_type_code::_sint32 ){
		retval = ConstantInt::get( extract_common_ctxt( c_si->type_info() )->type, uint64_t( c_si->value<int32_t>() ), true );
	} else if ( c_si->value_type() == buildin_type_code::_uint32 ) {
		retval = ConstantInt::get( extract_common_ctxt( c_si->type_info() )->type, uint64_t( c_si->value<uint32_t>() ), false );
	} else if ( c_si->value_type() == buildin_type_code::_float ) {
		retval = ConstantFP::get( extract_common_ctxt( c_si->type_info() )->type, c_si->value<double>() );
	} else {
		EFLIB_ASSERT_UNIMPLEMENTED();
	}

	data_as_cgctxt_ptr()->val = retval;
	*get_common_ctxt(v) = *data_as_cgctxt_ptr();
}

SASL_VISIT_DEF_UNIMPL( identifier );
SASL_VISIT_DEF( variable_expression ){

	shared_ptr<symbol> declsym = data_as_cgctxt_ptr()->sym.lock()->find( v.var_name->str );
	assert( declsym && declsym->node() );

	data_as_cgctxt_ptr()->addr = extract_common_ctxt( declsym->node() )->addr;
	data_as_cgctxt_ptr()->type = extract_common_ctxt( declsym->node() )->type;
	if ( data_as_cgctxt_ptr()->addr ){
		data_as_cgctxt_ptr()->val = ctxt->builder()->CreateLoad( data_as_cgctxt_ptr()->addr, v.var_name->str.c_str() );
	} else {
		data_as_cgctxt_ptr()->val = extract_common_ctxt( declsym->node() )->val;
	}
	
	*get_common_ctxt(v) = *data_as_cgctxt_ptr();
}

// declaration & type specifier
SASL_VISIT_DEF_UNIMPL( initializer );
SASL_VISIT_DEF( expression_initializer ){
	any child_ctxt_init = *data;
	any child_ctxt;

	visit_child( child_ctxt, child_ctxt_init, v.init_expr );

	shared_ptr<type_info_si> init_tsi = extract_semantic_info<type_info_si>(v.handle());
	shared_ptr<type_info_si> var_tsi = extract_semantic_info<type_info_si>(data_as_cgctxt_ptr()->variable_to_fill.lock());

	if( init_tsi->entry_id() != var_tsi->entry_id() ){
		typeconv->convert( var_tsi->type_info(), v.init_expr );
	}

	data_as_cgctxt_ptr()->type = any_to_cgctxt_ptr( child_ctxt )->type;
	data_as_cgctxt_ptr()->val = any_to_cgctxt_ptr( child_ctxt )->val;

	*get_common_ctxt(v) = *data_as_cgctxt_ptr();
}

SASL_VISIT_DEF_UNIMPL( member_initializer );
SASL_VISIT_DEF_UNIMPL( declaration );
SASL_VISIT_DEF( declarator ){
	any child_ctxt_init = *data;
	any_to_cgctxt_ptr( child_ctxt_init )->variable_to_fill = v.handle();
	any child_ctxt;

	Function* parent_func = data_as_cgctxt_ptr()->parent_func;

	IRBuilder<> vardecl_builder( ctxt->context() ) ;

	if( parent_func ){
		vardecl_builder.SetInsertPoint( &parent_func->getEntryBlock(), parent_func->getEntryBlock().begin() );
		data_as_cgctxt_ptr()->addr 
			= vardecl_builder.CreateAlloca( data_as_cgctxt_ptr()->type, 0, v.name->str.c_str() );
	} else {
		data_as_cgctxt_ptr()->addr
			= ctxt->module()->getOrInsertGlobal( v.name->str.c_str(), data_as_cgctxt_ptr()->type );
	}

	if ( v.init ){
		if (parent_func){
			visit_child( child_ctxt, child_ctxt_init, v.init );

			assert( any_to_cgctxt_ptr(child_ctxt)->val );
			vardecl_builder.CreateStore( any_to_cgctxt_ptr(child_ctxt)->val, data_as_cgctxt_ptr()->addr );
		} else {
			// Here is global variable initialization.
			EFLIB_ASSERT_UNIMPLEMENTED();
		}
	}

	*get_common_ctxt(v) = *data_as_cgctxt_ptr();
}
SASL_VISIT_DEF( variable_declaration ){
	any child_ctxt_init = *data;
	any child_ctxt;

	visit_child( child_ctxt, child_ctxt_init, v.type_info );
	assert( any_to_cgctxt_ptr(child_ctxt)->type );

	any_to_cgctxt_ptr(child_ctxt_init)->type = any_to_cgctxt_ptr(child_ctxt)->type;

	BOOST_FOREACH( shared_ptr<declarator> decl, v.declarators ){
		visit_child( child_ctxt, child_ctxt_init, decl );
	}

	*get_common_ctxt(v) = *data_as_cgctxt_ptr();
}

SASL_VISIT_DEF_UNIMPL( type_definition );
SASL_VISIT_DEF_UNIMPL( type_specifier );
SASL_VISIT_DEF( buildin_type ){

	shared_ptr<type_info_si> tisi = extract_semantic_info<type_info_si>( v );

	if ( tisi->type_info()->codegen_ctxt() ){
		*data = *extract_common_ctxt( tisi->type_info() );
		return;
	}

	const Type* ret_type = NULL;
	bool sign = false;

	if ( sasl_ehelper::is_void( v.value_typecode ) ){
		ret_type = Type::getVoidTy( ctxt->context() );
	} else if( sasl_ehelper::is_scalar(v.value_typecode) ){
		if( v.value_typecode == buildin_type_code::_boolean ){
			ret_type = IntegerType::get( ctxt->context(), 1 );
		} else if( sasl_ehelper::is_integer(v.value_typecode) ){
			ret_type = IntegerType::get( ctxt->context(), (unsigned int)sasl_ehelper::storage_size( v.value_typecode ) << 3 );
			sign = sasl_ehelper::is_signed( v.value_typecode );
		} else if ( v.value_typecode == buildin_type_code::_float ){
			ret_type = Type::getFloatTy( ctxt->context() );
		} else if ( v.value_typecode == buildin_type_code::_double ){
			ret_type = Type::getDoubleTy( ctxt->context() );
		}
	} else if( sasl_ehelper::is_vector( v.value_typecode) ){
		buildin_type_code btc = sasl_ehelper::scalar_of( v.value_typecode );
		any child_ctxt = *data_as_cgctxt_ptr();
		type_entry::id_t inner_typeid = gsi->type_manager()->get( btc );
		visit_child( child_ctxt, gsi->type_manager()->get(inner_typeid) );
		Type const* inner_type = any_to_cgctxt_ptr(child_ctxt)->type;
		ret_type = VectorType::get( inner_type, sasl_ehelper::len_0(v.value_typecode) );
	} else if( sasl_ehelper::is_matrix( v.value_typecode ) ){
		buildin_type_code btc = sasl_ehelper::scalar_of( v.value_typecode );
		any child_ctxt = *data_as_cgctxt_ptr();
		type_entry::id_t inner_typeid = gsi->type_manager()->get( btc );
		visit_child( child_ctxt, gsi->type_manager()->get(inner_typeid) );
		Type const* inner_type = any_to_cgctxt_ptr(child_ctxt)->type;
		Type const* row_type = VectorType::get( inner_type,sasl_ehelper::len_1(v.value_typecode) );
		ret_type = ArrayType::get( inner_type, sasl_ehelper::len_0(v.value_typecode) );
	}

	std::string tips = v.value_typecode.name() + std::string(" was not supported yet.");
	EFLIB_ASSERT_AND_IF( ret_type, tips.c_str() ){
		return;
	}

	data_as_cgctxt_ptr()->type = ret_type;
	data_as_cgctxt_ptr()->is_signed = sign;

	*get_common_ctxt( tisi->type_info() ) = *(data_as_cgctxt_ptr());
}

SASL_VISIT_DEF_UNIMPL( array_type );
SASL_VISIT_DEF_UNIMPL( struct_type );
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
		visit_child( child_ctxt, child_ctxt_init, *it );
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
		common_ctxt_handle par_ctxt = extract_common_ctxt( par );
		par_ctxt->val = arg_it;
	}


	// Create function body.
	if ( v.body ){
		visit_child( child_ctxt, child_ctxt_init, v.body );

		//////////////////////////////////////////////////////////////////////////
		// Process empty block.

		// Inner empty block, insert an br instruction for jumping to next block.
		for( Function::BasicBlockListType::iterator it = fn->getBasicBlockList().begin();
			it != fn->getBasicBlockList().end(); ++it
			)
		{
			if( it->empty() ){
				Function::BasicBlockListType::iterator next_it = it;
				++next_it;

				if( next_it != fn->getBasicBlockList().end() ){
					ctxt->builder()->CreateBr( &(*next_it) );
				} else {
					Value* val = get_zero_filled_constant( v.retval_type );
					ctxt->builder()->CreateRet(val);
				}
			}
		}
	}

	*get_common_ctxt(v) = *( data_as_cgctxt_ptr() );
}

// statement
SASL_VISIT_DEF_UNIMPL( statement );
SASL_VISIT_DEF( declaration_statement ){
	any child_ctxt_init = *data;
	any child_ctxt;

	visit_child( child_ctxt, child_ctxt_init, v.decl );

	*get_common_ctxt(v) = *data_as_cgctxt_ptr();
}
SASL_VISIT_DEF( if_statement ){
	any child_ctxt_init = *data;
	any child_ctxt;

	visit_child( child_ctxt, child_ctxt_init, v.cond );
	type_entry::id_t cond_tid = extract_semantic_info<type_info_si>(v.cond)->entry_id();
	type_entry::id_t bool_tid = gsi->type_manager()->get( buildin_type_code::_boolean );
	if( cond_tid != bool_tid ){
		typeconv->convert( gsi->type_manager()->get(bool_tid), v.cond );
	}
	BasicBlock* cond_block = ctxt->builder()->GetInsertBlock();

	// Generate 'then' branch code.
	BasicBlock* yes_block = BasicBlock::Create( ctxt->context(), v.yes_stmt->symbol()->mangled_name(), data_as_cgctxt_ptr()->parent_func );
	ctxt->builder()->SetInsertPoint( yes_block );
	visit_child( child_ctxt, child_ctxt_init, v.yes_stmt );
	BasicBlock* after_yes_block = ctxt->builder()->GetInsertBlock();
	
	// Generate 'else' branch code.
	BasicBlock* no_block = NULL;
	if( v.no_stmt ){
		no_block = BasicBlock::Create( ctxt->context(), v.no_stmt->symbol()->mangled_name(), data_as_cgctxt_ptr()->parent_func );
		ctxt->builder()->SetInsertPoint( no_block );
		visit_child( child_ctxt, child_ctxt_init, v.no_stmt );
	}
	BasicBlock* after_no_block = ctxt->builder()->GetInsertBlock();

	// Generate aggragate block
	BasicBlock* aggregate_block = BasicBlock::Create(
			ctxt->context(),
			extract_semantic_info<statement_si>(v)->exit_point().c_str(),
			data_as_cgctxt_ptr()->parent_func
		);
	
	// Fill back if-jump instruction
	ctxt->builder()->SetInsertPoint( cond_block );
	ctxt->builder()->CreateCondBr( extract_common_ctxt(v.cond)->val, yes_block, no_block ? no_block : aggregate_block );

	// Fill back jump out instruct of each branch.
	ctxt->builder()->SetInsertPoint( after_yes_block );
	ctxt->builder()->CreateBr( aggregate_block );
	
	ctxt->builder()->SetInsertPoint( after_no_block );
	ctxt->builder()->CreateBr( aggregate_block );

	// Set insert point to end of code.
	ctxt->builder()->SetInsertPoint( aggregate_block );

	*get_common_ctxt(v) = *data_as_cgctxt_ptr();
}

SASL_VISIT_DEF_UNIMPL( while_statement );
SASL_VISIT_DEF_UNIMPL( dowhile_statement );
SASL_VISIT_DEF_UNIMPL( case_label );
SASL_VISIT_DEF_UNIMPL( switch_statement );

SASL_VISIT_DEF( compound_statement ){
	data_as_cgctxt_ptr()->sym = v.symbol();

	any child_ctxt_init = *data;
	any child_ctxt;

	BasicBlock* bb = NULL;
	// If instruction block is the first block of function, we must create it.
	if ( data_as_cgctxt_ptr()->parent_func->getBasicBlockList().empty() ){
		bb = BasicBlock::Create(
				ctxt->context(),
				v.symbol()->mangled_name(),
				data_as_cgctxt_ptr()->parent_func
				);
	}

	data_as_cgctxt_ptr()->block = bb;

	if(bb){
		ctxt->builder()->SetInsertPoint(bb);
	}

	for ( std::vector< boost::shared_ptr<statement> >::iterator it = v.stmts.begin();
		it != v.stmts.end(); ++it)
	{
		visit_child( child_ctxt, child_ctxt_init, *it );
	}

	*get_common_ctxt(v) = *( data_as_cgctxt_ptr() );
}

SASL_VISIT_DEF( expression_statement ){
	any child_ctxt_init = *data;
	any child_ctxt;

	visit_child( child_ctxt, child_ctxt_init, v.expr );

	*get_common_ctxt(v) = *( data_as_cgctxt_ptr() );
}

SASL_VISIT_DEF( jump_statement ){

	any child_ctxt_init = *data;
	any child_ctxt;

	if (v.jump_expr){
		visit_child( child_ctxt, child_ctxt_init, v.jump_expr );
	}

	if ( v.code == jump_mode::_return ){
		if ( !v.jump_expr ){
			data_as_cgctxt_ptr()->return_inst = ctxt->builder()->CreateRetVoid();
		} else {
			data_as_cgctxt_ptr()->return_inst = ctxt->builder()->CreateRet( any_to_cgctxt_ptr(child_ctxt)->val );
		}
	} else if ( v.code == jump_mode::_continue ){
		assert( data_as_cgctxt_ptr()->continue_to );
		ctxt->builder()->CreateBr( data_as_cgctxt_ptr()->continue_to );
	} else if ( v.code == jump_mode::_break ){
		assert( data_as_cgctxt_ptr()->break_to );
		ctxt->builder()->CreateBr( data_as_cgctxt_ptr()->break_to );
	}

	// Restart a new block for sealing the old block.
	restart_block(data);

	*get_common_ctxt(v) = *data_as_cgctxt_ptr();
}

SASL_VISIT_DEF_UNIMPL( ident_label );

SASL_VISIT_DEF( program ){
	UNREF_PARAM( data );
	if ( ctxt ){
		return;
	}

	ctxt = create_codegen_context<cgllvm_global_context>(v.handle());
	ctxt->create_module( v.name );

	typeconv = create_type_converter( ctxt->builder() );
	register_buildin_typeconv( typeconv, gsi->type_manager() );

	any child_ctxt = cgllvm_common_context();

	vector<Function*> proc_fns;

	for( vector< boost::shared_ptr<declaration> >::iterator
		it = v.decls.begin(); it != v.decls.end(); ++it )
	{
		visit_child( child_ctxt, (*it) );
	}
}

SASL_VISIT_DEF_UNIMPL( for_statement );

boost::shared_ptr<llvm_code> llvm_code_generator::generated_module(){
	return boost::shared_polymorphic_cast<llvm_code>(ctxt);
}

void llvm_code_generator::global_semantic_info( boost::shared_ptr< sasl::semantic::global_si > v )
{
	gsi = v;
}
END_NS_SASL_CODE_GENERATOR();
