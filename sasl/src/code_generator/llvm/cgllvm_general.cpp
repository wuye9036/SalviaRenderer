#include <sasl/include/code_generator/llvm/cgllvm_general.h>

#include <sasl/enums/enums_helper.h>
#include <sasl/include/code_generator/llvm/cgllvm_contexts.h>
#include <sasl/include/code_generator/llvm/cgllvm_impl.imp.h>
#include <sasl/include/code_generator/llvm/cgllvm_globalctxt.h>
#include <sasl/include/code_generator/llvm/cgllvm_type_converters.h>
#include <sasl/include/semantic/name_mangler.h>
#include <sasl/include/semantic/abi_analyser.h>
#include <sasl/include/semantic/semantic_infos.imp.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/semantic/type_checker.h>
#include <sasl/include/semantic/type_converter.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/expression.h>
#include <sasl/include/syntax_tree/statement.h>
#include <sasl/include/syntax_tree/program.h>

#include <softart/include/enums.h>

#include <eflib/include/diagnostics/assert.h>
#include <eflib/include/metaprog/util.h>
#include <eflib/include/platform/boost_begin.h>
#include <boost/assign/std/vector.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <eflib/include/platform/boost_end.h>

#include <string>

BEGIN_NS_SASL_CODE_GENERATOR();

using namespace syntax_tree;
using namespace boost::assign;
using namespace llvm;

using semantic::abi_info;
using semantic::const_value_si;
using semantic::extract_semantic_info;
using semantic::module_si;
using semantic::storage_si;
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

typedef cgllvm_sctxt* sctxt_handle;

#define is_node_class( handle_of_node, typecode ) ( (handle_of_node)->node_class() == syntax_node_types::typecode )

//////////////////////////////////////////////////////////////////////////
//
#define SASL_VISITOR_TYPE_NAME cgllvm_general

cgllvm_general::cgllvm_general()
{}

// Process assign
void cgllvm_general::do_assign( any* data, shared_ptr<expression> lexpr, shared_ptr<expression> rexpr )
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

	store( load( node_ctxt(rexpr) ), node_ctxt(lexpr) );

	sc_data_ptr(data)->val_type = node_ctxt(lexpr)->data().val_type;
	sc_data_ptr(data)->local = node_ctxt(lexpr)->data().local;
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
		node_ctxt(v, true)->data().val_type = node_ctxt(v.casted_type)->data().val_type;
		typeconv->convert( v.handle(), v.expr );
	}

	cgllvm_sctxt* vctxt = node_ctxt(v, false);
	sc_data_ptr(data)->val_type = vctxt->data().val_type;
	sc_data_ptr(data)->val = load( vctxt );
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
			= sc_env_ptr(data)->sym.lock()->find_overloads( operator_name( v.op ), typeconv, args );

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
			if( ! node_ctxt( p0_tsi->type_info() ) ){
				visit_child( child_ctxt, child_ctxt_init,  op_proto->params[0]->param_type );
			}
			node_ctxt(v.left_expr)->data().val_type = node_ctxt( p0_tsi->type_info() )->data().val_type;
			typeconv->convert( p0_tsi->type_info(), v.left_expr );
		}
		if( p1_tsi->entry_id() != rarg_tsi->entry_id() ){
			if( ! node_ctxt( p1_tsi->type_info() ) ){
				visit_child( child_ctxt, child_ctxt_init, op_proto->params[1]->param_type );
			}
			node_ctxt(v.right_expr)->data().val_type = node_ctxt( p1_tsi->type_info() )->data().val_type;
			typeconv->convert( p1_tsi->type_info(), v.right_expr );
		}

		// use type-converted value to generate code.
		Value* lval = load( node_ctxt( v.left_expr ) );
		Value* rval = load( node_ctxt( v.right_expr ) );

		Value* retval = NULL;
		if( lval && rval ){

			builtin_type_code lbtc = p0_tsi->type_info()->value_typecode;
			builtin_type_code rbtc = p1_tsi->type_info()->value_typecode;

			if (v.op == operators::add){
				if( sasl_ehelper::is_real(lbtc) ){
					retval = mod_ptr()->builder()->CreateFAdd( lval, rval, "" );
				} else if( sasl_ehelper::is_integer(lbtc) ){
					retval = mod_ptr()->builder()->CreateAdd( lval, rval, "" );
				}
			} else if ( v.op == operators::sub ){
				if( sasl_ehelper::is_real(lbtc) ){
					retval = mod_ptr()->builder()->CreateFSub( lval, rval, "" );
				} else if( sasl_ehelper::is_integer(lbtc) ){
					retval = mod_ptr()->builder()->CreateSub( lval, rval, "" );
				}
			} else if ( v.op == operators::mul ){
				if( sasl_ehelper::is_real(lbtc) ){
					retval = mod_ptr()->builder()->CreateFMul( lval, rval, "" );
				} else if( sasl_ehelper::is_integer(lbtc) ){
					retval = mod_ptr()->builder()->CreateMul( lval, rval, "" );
				}
			} else if ( v.op == operators::div ){
				if( sasl_ehelper::is_real(lbtc) ){
					retval = mod_ptr()->builder()->CreateFDiv( lval, rval, "" );
				} else if( sasl_ehelper::is_integer(lbtc) ){
					// TODO support signed integer yet.
					retval = mod_ptr()->builder()->CreateSDiv( lval, rval, "" );
				}
			} else if ( v.op == operators::less ){
				if(sasl_ehelper::is_real(lbtc)){
					retval = mod_ptr()->builder()->CreateFCmpULT( lval, rval );
				} else if ( sasl_ehelper::is_integer(lbtc) ){
					if( sasl_ehelper::is_signed(lbtc) ){
						retval = mod_ptr()->builder()->CreateICmpSLT(lval, rval);
					}
					if( sasl_ehelper::is_unsigned(lbtc) ){
						retval = mod_ptr()->builder()->CreateICmpULT(lval, rval);
					}
				}
			} else {
				EFLIB_INTERRUPT( (boost::format("Operator %s is not supported yet.") % v.op.name() ).str().c_str() );
			}
		}

		store( retval, sc_ptr(data) );
	}

	node_ctxt(v, true)->copy( sc_ptr(data) );
}

SASL_VISIT_DEF_UNIMPL( expression_list );
SASL_VISIT_DEF_UNIMPL( cond_expression );
SASL_VISIT_DEF_UNIMPL( index_expression );
SASL_VISIT_DEF_UNIMPL( call_expression );

SASL_VISIT_DEF( constant_expression ){

	any child_ctxt_init = *data;
	any child_ctxt;

	boost::shared_ptr<const_value_si> c_si = extract_semantic_info<const_value_si>(v);
	if( ! node_ctxt( c_si->type_info() ) ){
		visit_child( child_ctxt, child_ctxt_init, c_si->type_info() );
	}

	Value* retval = NULL;

	cgllvm_sctxt* const_ctxt = node_ctxt( c_si->type_info() );
	Type const* const_lltype = const_ctxt->data().val_type;

	if( c_si->value_type() == builtin_type_code::_sint32 ){
		retval = ConstantInt::get( const_lltype, uint64_t( c_si->value<int32_t>() ), true );
	} else if ( c_si->value_type() == builtin_type_code::_uint32 ) {
		retval = ConstantInt::get( const_lltype, uint64_t( c_si->value<uint32_t>() ), false );
	} else if ( c_si->value_type() == builtin_type_code::_float ) {
		retval = ConstantFP::get( const_lltype, c_si->value<double>() );
	} else {
		EFLIB_ASSERT_UNIMPLEMENTED();
	}

	store( retval, sc_ptr(data) );

	node_ctxt(v, true)->copy( sc_ptr(data) );
}

SASL_VISIT_DEF_UNIMPL( identifier );

// declaration & type specifier
SASL_VISIT_DEF_UNIMPL( initializer );
SASL_VISIT_DEF( expression_initializer ){
	any child_ctxt_init = *data;
	any child_ctxt;

	visit_child( child_ctxt, child_ctxt_init, v.init_expr );

	shared_ptr<type_info_si> init_tsi = extract_semantic_info<type_info_si>(v.handle());
	shared_ptr<type_info_si> var_tsi = extract_semantic_info<type_info_si>(sc_env_ptr(data)->variable_to_fill.lock());

	if( init_tsi->entry_id() != var_tsi->entry_id() ){
		typeconv->convert( var_tsi->type_info(), v.init_expr );
	}

	sc_ptr(data)->storage_and_type( sc_ptr(child_ctxt) );
	node_ctxt(v, true)->copy( sc_ptr(data) );
}

SASL_VISIT_DEF_UNIMPL( member_initializer );
SASL_VISIT_DEF_UNIMPL( type_definition );
SASL_VISIT_DEF_UNIMPL( type_specifier );


SASL_VISIT_DEF_UNIMPL( array_type );
SASL_VISIT_DEF_UNIMPL( alias_type );
SASL_VISIT_DEF( parameter ){

	sc_env_ptr(data)->sym = v.symbol();

	any child_ctxt_init = *data;
	any child_ctxt;

	visit_child( child_ctxt, child_ctxt_init, v.param_type );
	sc_ptr(data)->type( sc_ptr(child_ctxt) );
	if (v.init){
		visit_child( child_ctxt, child_ctxt_init, v.init );
	} 

	node_ctxt(v, true)->copy( sc_ptr(data) );

}

SASL_VISIT_DEF( function_type ){
	parent_class::visit(v, data);
}

// statement
SASL_VISIT_DEF_UNIMPL( statement );

SASL_VISIT_DEF( if_statement ){
	any child_ctxt_init = *data;
	any child_ctxt;

	visit_child( child_ctxt, child_ctxt_init, v.cond );
	type_entry::id_t cond_tid = extract_semantic_info<type_info_si>(v.cond)->entry_id();
	type_entry::id_t bool_tid = msi->type_manager()->get( builtin_type_code::_boolean );
	if( cond_tid != bool_tid ){
		typeconv->convert( msi->type_manager()->get(bool_tid), v.cond );
	}
	BasicBlock* cond_block = mod_ptr()->builder()->GetInsertBlock();

	// Generate 'then' branch code.
	BasicBlock* yes_block = BasicBlock::Create( mod_ptr()->context(), v.yes_stmt->symbol()->mangled_name(), sc_env_ptr(data)->parent_fn );
	mod_ptr()->builder()->SetInsertPoint( yes_block );
	visit_child( child_ctxt, child_ctxt_init, v.yes_stmt );
	BasicBlock* after_yes_block = mod_ptr()->builder()->GetInsertBlock();
	
	// Generate 'else' branch code.
	BasicBlock* no_block = NULL;
	if( v.no_stmt ){
		no_block = BasicBlock::Create( mod_ptr()->context(), v.no_stmt->symbol()->mangled_name(), sc_env_ptr(data)->parent_fn );
		mod_ptr()->builder()->SetInsertPoint( no_block );
		visit_child( child_ctxt, child_ctxt_init, v.no_stmt );
	}
	BasicBlock* after_no_block = mod_ptr()->builder()->GetInsertBlock();

	// Generate aggragate block
	BasicBlock* aggregate_block = BasicBlock::Create(
			mod_ptr()->context(),
			extract_semantic_info<statement_si>(v)->exit_point().c_str(),
			sc_env_ptr(data)->parent_fn
		);
	
	// Fill back if-jump instruction
	builder()->SetInsertPoint( cond_block );
	builder()->CreateCondBr( load( node_ctxt(v.cond) ), yes_block, no_block ? no_block : aggregate_block );

	// Fill back jump out instruct of each branch.
	builder()->SetInsertPoint( after_yes_block );
	builder()->CreateBr( aggregate_block );
	
	builder()->SetInsertPoint( after_no_block );
	builder()->CreateBr( aggregate_block );

	// Set insert point to end of code.
	builder()->SetInsertPoint( aggregate_block );

	node_ctxt(v, true)->copy( sc_ptr(data) );
}

SASL_VISIT_DEF_UNIMPL( while_statement );
SASL_VISIT_DEF_UNIMPL( dowhile_statement );
SASL_VISIT_DEF_UNIMPL( case_label );
SASL_VISIT_DEF_UNIMPL( switch_statement );

SASL_VISIT_DEF_UNIMPL( ident_label );

SASL_VISIT_DEF_UNIMPL( for_statement );

cgllvm_modimpl* cgllvm_general::mod_ptr(){
	assert( dynamic_cast<cgllvm_modimpl*>( mod.get() ) );
	return static_cast<cgllvm_modimpl*>( mod.get() );
}

bool cgllvm_general::create_mod( sasl::syntax_tree::program& v ){
	if ( mod ){ return false; }
	mod = create_codegen_context<cgllvm_modimpl>( v.handle() );
	return true;
}

END_NS_SASL_CODE_GENERATOR();
