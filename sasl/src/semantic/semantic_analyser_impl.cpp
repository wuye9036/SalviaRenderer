#include <sasl/include/semantic/semantic_analyser_impl.h>

#include <sasl/enums/operators.h>
#include <sasl/include/common/compiler_info_manager.h>
#include <sasl/include/semantic/name_mangler.h>
#include <sasl/include/semantic/semantic_error.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/semantic/semantic_infos.h>
#include <sasl/include/semantic/symbol_scope.h>
#include <sasl/include/semantic/type_checker.h>
#include <sasl/include/semantic/type_converter.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/expression.h>
#include <sasl/include/syntax_tree/make_tree.h>
#include <sasl/include/syntax_tree/program.h>
#include <sasl/include/syntax_tree/statement.h>
#include <sasl/include/syntax_tree/utility.h>

#include <eflib/include/diagnostics/assert.h>

#include <boost/assign/list_of.hpp>
#include <boost/assign/list_inserter.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/bind.hpp>
#include <boost/bind/apply.hpp>
#include <boost/scoped_ptr.hpp>

BEGIN_NS_SASL_SEMANTIC();

using ::sasl::common::compiler_info_manager;
using ::sasl::common::token_attr;

using ::sasl::syntax_tree::binary_expression;
using ::sasl::syntax_tree::buildin_type;
using ::sasl::syntax_tree::create_buildin_type;
using ::sasl::syntax_tree::create_node;
using ::sasl::syntax_tree::declaration;
using ::sasl::syntax_tree::expression;
using ::sasl::syntax_tree::function_type;
using ::sasl::syntax_tree::node;
using ::sasl::syntax_tree::parameter;
using ::sasl::syntax_tree::program;
using ::sasl::syntax_tree::statement;
using ::sasl::syntax_tree::type_specifier;

using ::sasl::syntax_tree::dfunction_combinator;

using ::sasl::semantic::errors::semantic_error;
using ::sasl::semantic::extract_semantic_info;
using ::sasl::semantic::program_si;
using ::sasl::semantic::symbol;

using namespace std;
using namespace boost::assign;

// utility functions

boost::shared_ptr<type_specifier> type_info_of( boost::shared_ptr<node> n ){
	boost::shared_ptr<type_info_si> typesi = extract_semantic_info<type_info_si>( n );
	if ( typesi ){
		return typesi->type_info();
	}
	return boost::shared_ptr<type_specifier>();
}

semantic_analyser_impl::semantic_analyser_impl( boost::shared_ptr<compiler_info_manager> infomgr )
	: infomgr( infomgr )
{
	typeconv.reset( new type_converter() );
	register_type_converter();
}

#define SASL_VISITOR_TYPE_NAME semantic_analyser_impl

SASL_VISIT_NOIMPL( unary_expression );
SASL_VISIT_NOIMPL( cast_expression );
SASL_VISIT_DEF( binary_expression )
{
	v.left_expr->accept(this, data);
	v.right_expr->accept(this, data);

	// TODO: look up operator prototype.
	std::string opname = operator_name( v.op );
	vector< boost::shared_ptr<expression> > exprs;
	exprs += v.left_expr, v.right_expr;
	vector< boost::shared_ptr<symbol> > overloads = cursym->find_overloads( opname, typeconv, exprs );

	EFLIB_ASSERT_AND_IF( !overloads.empty(), "Need to report a compiler error. No overloading." ){
		return;
	}
	EFLIB_ASSERT_AND_IF( overloads.size() == 1, "Need to report a compiler error. Ambigous overloading." ){
		return;
	}
}

SASL_VISIT_NOIMPL( expression_list );
SASL_VISIT_NOIMPL( cond_expression );
SASL_VISIT_NOIMPL( index_expression );
SASL_VISIT_NOIMPL( call_expression );
SASL_VISIT_NOIMPL( member_expression );

SASL_VISIT_DEF( constant_expression )
{
	using ::sasl::syntax_tree::constant_expression;

	boost::shared_ptr<const_value_si> vseminfo = get_or_create_semantic_info<const_value_si>(v);
	vseminfo->set_literal( v.value_tok->str, v.ctype );
}

SASL_VISIT_NOIMPL( variable_expression );

// declaration & type specifier
SASL_VISIT_NOIMPL( initializer );
SASL_VISIT_DEF( expression_initializer )
{
	v.init_expr->accept(this, data);
}

SASL_VISIT_NOIMPL( member_initializer );
SASL_VISIT_NOIMPL( declaration );

SASL_VISIT_DEF( variable_declaration )
{
	using ::boost::assign::list_of;

	symbol_scope sc( v.name->str, v.handle(), cursym );

	// process variable type
	boost::shared_ptr<type_specifier> vartype = v.type_info;
	vartype->accept( this, data );
	boost::shared_ptr<type_semantic_info> typeseminfo = extract_semantic_info<type_semantic_info>(v);
	
	// check type.
	if ( typeseminfo->type_type() == type_types::buildin ){
		// TODO: ALLOCATE BUILD-IN TYPED VAR.
	} else if ( typeseminfo->type_type() == type_types::composited ){
		// TODO: ALLOCATE COMPOSITED TYPED VAR.
	} else if ( typeseminfo->type_type() == type_types::alias ){
		if ( typeseminfo->full_type() ){
			// TODO: ALLOCATE ACTUAL
		} else {
			infomgr->add_info( semantic_error::create( compiler_informations::uses_a_undef_type,
				v.handle(), list_of( typeseminfo->full_type() ) )
				);
			// remove created symbol
			cursym->remove();
			return;
		}
	}

	// process initializer
	v.init->accept( this, data );;
}

SASL_VISIT_DEF( type_definition ){
	using ::sasl::syntax_tree::type_definition;
	using ::boost::assign::list_of;
	const std::string& alias_str = v.name->str;
	boost::shared_ptr<symbol> existed_sym = cursym->find( alias_str );
	if ( existed_sym ){
		// if the symbol is used and is not a type node, it must be redifinition.
		// else compare the type.
		if ( !existed_sym->node()->node_class().included( syntax_node_types::type_specifier ) ){
			infomgr->add_info( 
				semantic_error::create( compiler_informations::redef_cannot_overloaded,
				v.handle(),	list_of(existed_sym->node()) )
					);
			return;
		}
	}

	// process type node.
	// remove old sym from symbol table.
	cursym->remove_child( v.name->str );
	{
		symbol_scope sc( v.name->str, v.handle(), cursym );

		v.type_info->accept(this, data);
		boost::shared_ptr<type_semantic_info> new_tsi = extract_semantic_info<type_semantic_info>(v);

		// if this symbol is usable, process type node.
		if ( existed_sym ){
			boost::shared_ptr<type_semantic_info> existed_tsi = extract_semantic_info<type_semantic_info>( existed_sym->node() );
			if ( !type_equal(existed_tsi->full_type(), new_tsi->full_type()) ){
				// if new symbol is different from the old, semantic error.
				// The final effect is that the new definition overwrites the old one.

				infomgr->add_info( 
					semantic_error::create( compiler_informations::redef_diff_basic_type,
					v.handle(),	list_of(existed_sym->node()) )
					);
			} 
		}
		// else if the same. do not updated.
		// NOTE:
		//   MAYBE IT NEEDS COMBINE OLD AND NEW SYMBOL INFOS UNDER SOME CONDITIONS. 
		//   BUT I CAN NOT FIND OUT ANY EXAMPLE.
	}
}

SASL_VISIT_NOIMPL( type_specifier );
SASL_VISIT_DEF( buildin_type ){
	using ::sasl::semantic::get_or_create_semantic_info;

	// create type information on current symbol.
	// for e.g. create type info onto a variable node.
	boost::shared_ptr<type_si> tsi = get_or_create_semantic_info<type_si>( v.handle() );
	tsi->type_info( v.typed_handle<type_specifier>() );
}

SASL_VISIT_NOIMPL( array_type );
SASL_VISIT_NOIMPL( struct_type );

SASL_VISIT_DEF( parameter )
{
	symbol_scope ss( v.name ? v.name->str : std::string(), v.handle(), cursym );
	v.param_type->accept( this, data );;
	if ( v.init ){
		v.init->accept( this, data );;
	}
	get_or_create_semantic_info<storage_si>(v)->type_info( type_info_si::from_node(v.param_type) );
}

SASL_VISIT_DEF( function_type )
{
	// TODO: add document for explaining why we need add_mangling().
	std::string name = v.name->str;
	symbol_scope ss( name, v.handle(), cursym );

	v.retval_type->accept( this, data );;
	for( vector< boost::shared_ptr<parameter> >::iterator it = v.params.begin();
		it != v.params.end(); ++it )
	{
		(*it)->accept( this, data );;
	}
	cursym->add_mangling( mangle( v.typed_handle<function_type>() ) );

	if ( v.body ){
		v.body->accept( this, data );;
	}
	// TODO : It's doing nothing now.

	//using ::sasl::semantic::symbol;
	//using ::sasl::semantic::get_or_create_semantic_info;
	//using ::sasl::semantic::extract_semantic_info;

	//// if it is only declaration.
	//std::string symbol_name;
	//std::string unmangled_name = v.name->str;

	//// process parameter types for name mangling.
	//v.retval_type->accept( this, data );;
	//for( size_t i_param = 0; i_param < v.params.size(); ++i_param ){
	//	v.params[i_param]->param_type->accept(this, data);
	//}

	//std::string mangled_name = mangle_function_name( v.typed_handle<function_type>() );

	//bool use_existed_node(false);

	//boost::shared_ptr<symbol> existed_sym = cursym->find_mangled_this( unmangled_name );
	//if ( existed_sym ) {
	//	boost::shared_ptr<function_type> existed_node = existed_sym->node()->typed_handle<function_type>();
	//	if ( !existed_node ){
	//		// symbol was used, and it is not a function. error.
	//		// TODO: SEMANTIC ERROR: TYPE REDEFINITION.
	//	} else {
	//		// symbol was used, and the older is a function.
	//		existed_node = cursym->find_mangled_this(mangled_name)->node()->typed_handle<function_type>();
	//		if ( existed_node ){
	//			if ( !is_equal( existed_node, v.typed_handle<function_type>() ) ){
	//				// TODO: BUG ON OVERLOAD SUPPORTING
	//				// TODO: SEMANTIC ERROR ON OVERLOAD UNSUPPORTED.
	//			}
	//			if ( v.declaration_only() ){
	//				// it was had a definition/declaration, and now is a declaration only
	//				use_existed_node = true;
	//			} else {
	//				if ( existed_node->declaration_only() ){
	//					// the older is a declaration, and whatever now is, that's OK.
	//					use_existed_node = true;
	//				} else {
	//					// older function definition v.s. new function definition, conflict...
	//					// TODO:  SEMANTIC ERROR REDEFINE A FUNCTION.
	//				}
	//			}
	//		} else {
	//			use_existed_node = false;
	//		}
	//	}
	//} else {
	//	use_existed_node = false;
	//}

	//boost::scoped_ptr<symbol_scope> sc(
	//	use_existed_node ? new symbol_scope(mangled_name, cursym) : new symbol_scope( mangled_name, unmangled_name, v.handle(), cursym )
	//	);

	//v.symbol( cursym );
	//if ( !use_existed_node ){
	//	// replace old node via new node.
	//	cursym->relink( v.handle() );
	//	
	//	// definition
	//	if ( !v.declaration_only() ){
	//		// process parameters
	//		for( size_t i_param = 0; i_param < v.params.size(); ++i_param ){
	//			v.params[i_param]->accept( this, data );;
	//		}

	//		// process statements
	//		is_local = true;
	//		for( size_t i_stmt = 0; i_stmt < v.body->stmts.size(); ++i_stmt ){
	//			v.body->stmts[i_stmt]->accept( this, data );;
	//		}
	//	}
	//}
}

// statement
SASL_VISIT_NOIMPL( statement );

SASL_VISIT_DEF( declaration_statement )
{
	v.decl->accept(this, data);
}

SASL_VISIT_DEF( if_statement )
{
	v.cond->accept( this, data );;
	v.yes_stmt->accept( this, data );;
	v.no_stmt->accept(this, data);
}

SASL_VISIT_DEF( while_statement ){
	v.cond->accept( this, data );;
	v.body->accept( this, data );;
}

SASL_VISIT_NOIMPL( dowhile_statement );
SASL_VISIT_NOIMPL( case_label );
SASL_VISIT_NOIMPL( ident_label );
SASL_VISIT_NOIMPL( switch_statement );

SASL_VISIT_DEF( compound_statement )
{
	{
		// add symbol to v. it can used by code block name.
		symbol_scope( std::string(""), v.handle(), cursym );
	}

	for( vector< boost::shared_ptr<statement> >::iterator it = v.stmts.begin();
		it != v.stmts.end(); ++it)
	{
		(*it)->accept(this, data);
	}
}

SASL_VISIT_NOIMPL( expression_statement );

SASL_VISIT_DEF( jump_statement )
{
	if (v.code == jump_mode::_return){
		if( v.jump_expr ){
			v.jump_expr->accept(this, data);
		}
	}
}

// program
SASL_VISIT_DEF( program ){
	// create semantic info
	boost::shared_ptr<program_si> sem = get_or_create_semantic_info<program_si>(v);
	sem->name( v.name );
	
	// create root symbol
	v.symbol( symbol::create_root( v.handle() ) );
	cursym = v.symbol();

	register_buildin_function( v );

	// analysis decalarations.
	for( vector< boost::shared_ptr<declaration> >::iterator it = v.decls.begin(); it != v.decls.end(); ++it ){
		(*it)->accept( this, data );;
	}
}

SASL_VISIT_NOIMPL( for_statement );

void semantic_analyser_impl::buildin_type_convert( boost::shared_ptr<node> lhs, boost::shared_ptr<node> rhs ){
	// do nothing
}

void semantic_analyser_impl::register_type_converter(){
	// register default type converter
	boost::shared_ptr<type_specifier> sint8_ts = create_buildin_type( buildin_type_code::_sint8 );
	boost::shared_ptr<type_specifier> sint16_ts = create_buildin_type( buildin_type_code::_sint16 );
	boost::shared_ptr<type_specifier> sint32_ts = create_buildin_type( buildin_type_code::_sint32 );
	boost::shared_ptr<type_specifier> sint64_ts = create_buildin_type( buildin_type_code::_sint64 );

	boost::shared_ptr<type_specifier> uint8_ts = create_buildin_type( buildin_type_code::_uint8 );
	boost::shared_ptr<type_specifier> uint16_ts = create_buildin_type( buildin_type_code::_uint16 );
	boost::shared_ptr<type_specifier> uint32_ts = create_buildin_type( buildin_type_code::_uint32 );
	boost::shared_ptr<type_specifier> uint64_ts = create_buildin_type( buildin_type_code::_uint64 );

	boost::shared_ptr<type_specifier> float_ts = create_buildin_type( buildin_type_code::_float );
	boost::shared_ptr<type_specifier> double_ts = create_buildin_type( buildin_type_code::_double );

	boost::shared_ptr<type_specifier> bool_ts = create_buildin_type( buildin_type_code::_boolean );

	// default conversation will do nothing.
	type_converter::converter_t default_conv = boost::bind(&semantic_analyser_impl::buildin_type_convert, this, _1, _2);

	typeconv->register_converter( type_converter::implicit_conv, sint8_ts, sint16_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, sint8_ts, sint32_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, sint8_ts, sint64_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, sint8_ts, uint8_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, sint8_ts, uint16_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, sint8_ts, uint32_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, sint8_ts, uint64_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, sint8_ts, float_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, sint8_ts, double_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, sint8_ts, bool_ts, default_conv );

	typeconv->register_converter( type_converter::explicit_conv, sint16_ts, sint8_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, sint16_ts, sint32_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, sint16_ts, sint64_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, sint16_ts, uint8_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, sint16_ts, uint16_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, sint16_ts, uint32_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, sint16_ts, uint64_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, sint16_ts, float_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, sint16_ts, double_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, sint16_ts, bool_ts, default_conv );

	typeconv->register_converter( type_converter::explicit_conv, sint32_ts, sint8_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, sint32_ts, sint16_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, sint32_ts, sint64_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, sint32_ts, uint8_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, sint32_ts, uint16_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, sint32_ts, uint32_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, sint32_ts, uint64_ts, default_conv );
	typeconv->register_converter( type_converter::warning_conv, sint32_ts, float_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, sint32_ts, double_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, sint32_ts, bool_ts, default_conv );

	typeconv->register_converter( type_converter::explicit_conv, sint64_ts, sint8_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, sint64_ts, sint16_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, sint64_ts, sint32_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, sint64_ts, uint8_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, sint64_ts, uint16_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, sint64_ts, uint32_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, sint64_ts, uint64_ts, default_conv );
	typeconv->register_converter( type_converter::warning_conv, sint64_ts, float_ts, default_conv );
	typeconv->register_converter( type_converter::warning_conv, sint64_ts, double_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, sint64_ts, bool_ts, default_conv );

	typeconv->register_converter( type_converter::explicit_conv, uint8_ts, sint8_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, uint8_ts, sint16_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, uint8_ts, sint32_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, uint8_ts, sint64_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, uint8_ts, uint16_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, uint8_ts, uint32_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, uint8_ts, uint64_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, uint8_ts, float_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, uint8_ts, double_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, uint8_ts, bool_ts, default_conv );

	typeconv->register_converter( type_converter::explicit_conv, uint16_ts, sint8_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, uint16_ts, sint16_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, uint16_ts, sint32_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, uint16_ts, sint64_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, uint16_ts, uint8_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, uint16_ts, uint32_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, uint16_ts, uint64_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, uint16_ts, float_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, uint16_ts, double_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, uint16_ts, bool_ts, default_conv );

	typeconv->register_converter( type_converter::explicit_conv, uint32_ts, sint8_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, uint32_ts, sint16_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, uint32_ts, sint32_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, uint32_ts, sint64_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, uint32_ts, uint8_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, uint32_ts, uint16_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, uint32_ts, uint64_ts, default_conv );
	typeconv->register_converter( type_converter::warning_conv, uint32_ts, float_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, uint32_ts, double_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, uint32_ts, bool_ts, default_conv );

	typeconv->register_converter( type_converter::explicit_conv, uint64_ts, sint8_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, uint64_ts, sint16_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, uint64_ts, sint32_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, uint64_ts, sint64_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, uint64_ts, uint8_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, uint64_ts, uint16_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, uint64_ts, uint32_ts, default_conv );
	typeconv->register_converter( type_converter::warning_conv, uint64_ts, float_ts, default_conv );
	typeconv->register_converter( type_converter::warning_conv, uint64_ts, double_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, uint64_ts, bool_ts, default_conv );

	typeconv->register_converter( type_converter::explicit_conv, float_ts, sint8_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, float_ts, sint16_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, float_ts, sint32_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, float_ts, sint64_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, float_ts, uint8_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, float_ts, uint16_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, float_ts, uint32_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, float_ts, uint64_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, float_ts, double_ts, default_conv );
	typeconv->register_converter( type_converter::warning_conv, float_ts, bool_ts, default_conv );

	typeconv->register_converter( type_converter::explicit_conv, double_ts, sint8_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, double_ts, sint16_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, double_ts, sint32_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, double_ts, sint64_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, double_ts, uint8_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, double_ts, uint16_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, double_ts, uint32_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, double_ts, uint64_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, double_ts, float_ts, default_conv );
	typeconv->register_converter( type_converter::warning_conv, double_ts, bool_ts, default_conv );

	typeconv->register_converter( type_converter::explicit_conv, double_ts, sint8_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, double_ts, sint16_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, double_ts, sint32_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, double_ts, sint64_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, double_ts, uint8_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, double_ts, uint16_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, double_ts, uint32_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, double_ts, uint64_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, double_ts, float_ts, default_conv );
	typeconv->register_converter( type_converter::warning_conv, double_ts, bool_ts, default_conv );

	typeconv->register_converter( type_converter::explicit_conv, bool_ts, sint8_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, bool_ts, sint16_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, bool_ts, sint32_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, bool_ts, sint64_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, bool_ts, uint8_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, bool_ts, uint16_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, bool_ts, uint32_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, bool_ts, uint64_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, bool_ts, float_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, bool_ts, double_ts, default_conv );
	
}

void semantic_analyser_impl::register_buildin_function( node& v ){
	// 
	::boost::any* data = NULL;
	::std::vector< ::boost::shared_ptr<node> >& buildin_functions( v.additionals() );

	typedef boost::unordered_map<buildin_type_code, boost::shared_ptr<buildin_type> > bt_table_t;
	bt_table_t standard_bttbl;
	bt_table_t storage_bttbl;
	map_of_buildin_type( standard_bttbl, &sasl_ehelper::is_standard );
	map_of_buildin_type( storage_bttbl, &sasl_ehelper::is_storagable );

	boost::shared_ptr<buildin_type> bt_bool = storage_bttbl[ buildin_type_code::_boolean ];
	boost::shared_ptr<buildin_type> bt_i32 = storage_bttbl[ buildin_type_code::_sint32 ];

	boost::shared_ptr<function_type> tmpft;

	// arithmetic operators
	vector<std::string> op_tbl;
	const vector<operators>& oplist = sasl_ehelper::list_of_operators();

	for( size_t i_op = 0; i_op < oplist.size(); ++i_op ){
		operators op = oplist[i_op];
		std::string op_name( operator_name(op) );

		if ( sasl_ehelper::is_arithmetic(op) ){
			for( bt_table_t::iterator it_type = standard_bttbl.begin(); it_type != standard_bttbl.end(); ++it_type ){
				dfunction_combinator(NULL).dname( op_name )
					.dreturntype().dnode( it_type->second ).end()
					.dparam().dtype().dnode( it_type->second ).end().end()
					.dparam().dtype().dnode( it_type->second ).end().end()
				.end( tmpft );

				if ( tmpft ){
					tmpft->accept(this, data);
					buildin_functions.push_back( tmpft );
				}
			}
		}

		if( sasl_ehelper::is_arith_assign(op) ){
			for( bt_table_t::iterator it_type = standard_bttbl.begin(); it_type != standard_bttbl.end(); ++it_type ){
				dfunction_combinator(NULL).dname( op_name )
					.dreturntype().dnode( it_type->second ).end()
					.dparam().dtype().dnode( it_type->second ).end().end()
					.dparam().dtype().dnode( it_type->second ).end().end()
					.end( tmpft );
				if ( tmpft ){
					tmpft->accept(this, data);
					buildin_functions.push_back( tmpft );
				}
			}
		}

		if( sasl_ehelper::is_relationship(op) ){
			
			for( bt_table_t::iterator it_type = standard_bttbl.begin(); it_type != standard_bttbl.end(); ++it_type ){
				dfunction_combinator(NULL).dname( op_name )
					.dreturntype().dnode( bt_bool ).end()
					.dparam().dtype().dnode( it_type->second ).end().end()
					.dparam().dtype().dnode( it_type->second ).end().end()
					.end( tmpft );
				if ( tmpft ){
					tmpft->accept(this, data);
					buildin_functions.push_back( tmpft );
				}
			}
		}

		if( sasl_ehelper::is_bit(op) || sasl_ehelper::is_bit_assign(op) ){
			for( bt_table_t::iterator it_type = standard_bttbl.begin(); it_type != standard_bttbl.end(); ++it_type ){
				if ( sasl_ehelper::is_integer(it_type->first) ){
					dfunction_combinator(NULL).dname( op_name )
						.dreturntype().dnode( it_type->second ).end()
						.dparam().dtype().dnode( it_type->second ).end().end()
						.dparam().dtype().dnode( it_type->second ).end().end()
					.end( tmpft );
					if ( tmpft ){
						tmpft->accept(this, data);
						buildin_functions.push_back( tmpft );
					}
				}
			}
		}

		if( sasl_ehelper::is_shift(op) || sasl_ehelper::is_shift_assign(op) ){
			for( bt_table_t::iterator it_type = standard_bttbl.begin(); it_type != standard_bttbl.end(); ++it_type ){
				if ( sasl_ehelper::is_integer(it_type->first) ){
					dfunction_combinator(NULL).dname( op_name )
						.dreturntype().dnode( it_type->second ).end()
						.dparam().dtype().dnode( it_type->second ).end().end()
						.dparam().dtype().dnode( bt_i32 ).end().end()
						.end( tmpft );
					if ( tmpft ){
						tmpft->accept(this, data);
						buildin_functions.push_back( tmpft );
					}
				}
			}
		}

		if( sasl_ehelper::is_bool_arith(op) ){
			dfunction_combinator(NULL).dname( op_name )
				.dreturntype().dnode( bt_bool ).end()
				.dparam().dtype().dnode( bt_bool ).end().end()
				.dparam().dtype().dnode( bt_bool ).end().end()
			.end( tmpft );
			if ( tmpft ){
				tmpft->accept(this, data);
				buildin_functions.push_back( tmpft );
			}
		}

		if( sasl_ehelper::is_prefix(op) || sasl_ehelper::is_postfix(op) || op == operators::positive ){
			for( bt_table_t::iterator it_type = standard_bttbl.begin(); it_type != standard_bttbl.end(); ++it_type ){
				if ( sasl_ehelper::is_integer(it_type->first) ){
					dfunction_combinator(NULL).dname( op_name )
						.dreturntype().dnode( it_type->second ).end()
						.dparam().dtype().dnode( it_type->second ).end().end()
						.end( tmpft );

					if ( tmpft ){
						tmpft->accept(this, data);
						buildin_functions.push_back( tmpft );
					}
				}
			}
		}

		if( op == operators::bit_not ){
			for( bt_table_t::iterator it_type = standard_bttbl.begin(); it_type != standard_bttbl.end(); ++it_type ){
				if ( sasl_ehelper::is_integer(it_type->first) ){
					dfunction_combinator(NULL).dname( op_name )
						.dreturntype().dnode( it_type->second ).end()
						.dparam().dtype().dnode( it_type->second ).end().end()
						.end( tmpft );

					if ( tmpft ){
						tmpft->accept(this, data);
						buildin_functions.push_back( tmpft );
					}
				}
			}
		}

		if( op == operators::logic_not ){
			dfunction_combinator(NULL).dname( op_name )
				.dreturntype().dnode( bt_bool ).end()
				.dparam().dtype().dnode( bt_bool ).end().end()
			.end( tmpft );

			if ( tmpft ){
				tmpft->accept(this, data);
				buildin_functions.push_back( tmpft );
			}
		}

		if( op == operators::negative ){
			for( bt_table_t::iterator it_type = standard_bttbl.begin(); it_type != standard_bttbl.end(); ++it_type ){
				if ( it_type->first != buildin_type_code::_uint64 ){
					dfunction_combinator(NULL).dname( op_name )
						.dreturntype().dnode( it_type->second ).end()
						.dparam().dtype().dnode( it_type->second ).end().end()
					.end( tmpft );

					if ( tmpft ){
						tmpft->accept(this, data);
						buildin_functions.push_back( tmpft );
					}
				}
			}
		}

		if ( op == operators::assign ){
			for( bt_table_t::iterator it_type = storage_bttbl.begin(); it_type != storage_bttbl.end(); ++it_type ){
				dfunction_combinator(NULL).dname( op_name )
					.dreturntype().dnode( it_type->second ).end()
					.dparam().dtype().dnode( it_type->second ).end().end()
					.dparam().dtype().dnode( it_type->second ).end().end()
				.end( tmpft );

				if ( tmpft ){
					tmpft->accept(this, data);
					buildin_functions.push_back( tmpft );
				}
			}
		}
	}
}

END_NS_SASL_SEMANTIC();