#include <boost/test/unit_test.hpp>
#include "test_utility.h"
#include <sasl/include/semantic/semantic_analyser.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/semantic/symbol_infos.h>
#include <sasl/include/semantic/name_mangler.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/expression.h>
#include <sasl/include/syntax_tree/node_creation.h>
#include <sasl/include/syntax_tree/program.h>
#include <sasl/include/syntax_tree/statement.h>
#include <sasl/include/common/token_attr.h>
#include <sasl/include/common/compiler_info_manager.h>
#include <string>

BOOST_AUTO_TEST_SUITE( semantic );

BOOST_AUTO_TEST_CASE( program_semantic ){
	using ::sasl::syntax_tree::program;
	using ::sasl::syntax_tree::create_node;
	using ::sasl::semantic::semantic_analysis;
	using ::sasl::common::compiler_info_manager;

	boost::shared_ptr<compiler_info_manager> cim = compiler_info_manager::create();

	std::string prog_name("test");
	boost::shared_ptr<program> prog = create_node<program>( prog_name );
	BOOST_CHECK( !prog->symbol() );

	semantic_analysis( prog, cim );
	BOOST_CHECK( prog->symbol() );
}

BOOST_AUTO_TEST_CASE( constant_expr_semantic ){
	using ::sasl::syntax_tree::program;
	using ::sasl::syntax_tree::declaration_statement;
	using ::sasl::syntax_tree::variable_declaration;
	using ::sasl::syntax_tree::constant_expression;
	using ::sasl::syntax_tree::create_node;
	using ::sasl::syntax_tree::buildin_type;
	using ::sasl::syntax_tree::expression_initializer;
	using ::sasl::semantic::semantic_analysis;
	using ::sasl::common::token_attr;
	using ::sasl::semantic::extract_symbol_info;
	using ::sasl::semantic::const_value_symbol_info;
	using ::sasl::common::compiler_info_manager;

	std::string prog_name("test");
	std::string var_name("test_var");
	
	std::string var_intval("1.0f");

	boost::shared_ptr<token_attr> nulltok;

	// create
	boost::shared_ptr<program> prog = create_node<program>( prog_name );
	boost::shared_ptr<declaration_statement> declstmt = create_node<declaration_statement>();
	boost::shared_ptr<variable_declaration> decl = create_node<variable_declaration>( nulltok );
	boost::shared_ptr<buildin_type> vartype = create_node<buildin_type>( nulltok );
	boost::shared_ptr<token_attr> name_tok( new token_attr(var_name.begin(), var_name.end()) );
	boost::shared_ptr<constant_expression> initexpr = create_node<constant_expression>( nulltok );
	boost::shared_ptr<expression_initializer> init = create_node<expression_initializer>( nulltok );
	boost::shared_ptr<token_attr> intval_tok( new token_attr(var_intval.begin(), var_intval.end()) );

	// assemble
	initexpr->ctype = literal_constant_types::real;
	initexpr->value_tok = intval_tok;
	vartype->value_typecode = buildin_type_code::_sint32;
	init->init_expr = initexpr;
	decl->init = init;
	decl->name = name_tok;
	decl->type_info = vartype;
	declstmt->decl = decl;
	prog->decls.push_back( declstmt );

	boost::shared_ptr<compiler_info_manager> cim = compiler_info_manager::create();

	// do semantic
	semantic_analysis( prog, cim );

	// check
	boost::shared_ptr<const_value_symbol_info> cvsi = extract_symbol_info<const_value_symbol_info>(decl);
	double val = cvsi->value<double>();

	BOOST_CHECK_EQUAL( (double)1.0f, val );
	BOOST_CHECK( cvsi->value_type() == buildin_type_code::_float );
}

BOOST_AUTO_TEST_CASE( type_definition_semantic ){
	using ::sasl::syntax_tree::program;
	using ::sasl::syntax_tree::create_node;
	using ::sasl::syntax_tree::type_definition;
	using ::sasl::syntax_tree::buildin_type;
	using ::sasl::syntax_tree::type_specifier;
	using ::sasl::common::token_attr;
	using ::sasl::semantic::type_symbol_info;
	using ::sasl::semantic::semantic_analysis;
	using ::sasl::syntax_tree::declaration;
	using ::sasl::syntax_tree::declaration_statement;
	using ::sasl::semantic::extract_symbol_info;
	using ::sasl::semantic::symbol;
	using ::sasl::common::compiler_info_manager;

	boost::shared_ptr<compiler_info_manager> cim = compiler_info_manager::create();

	boost::shared_ptr<token_attr> nulltok;

	std::string var_name_0("var0");
	std::string var_name_1("var1");

	boost::shared_ptr<token_attr> var0_tok( new token_attr() );
	var0_tok->lit = var_name_0;
	boost::shared_ptr<token_attr> var1_tok( new token_attr() );
	var1_tok->lit = var_name_1;
	
	boost::shared_ptr<program> prog = create_node<program>("test");
	boost::shared_ptr<buildin_type> bti32 = create_node<buildin_type>(nulltok);
	bti32->value_typecode = buildin_type_code::_sint32;
	boost::shared_ptr<buildin_type> bti32_a = create_node<buildin_type>(nulltok);
	bti32->value_typecode = buildin_type_code::_sint32;
	boost::shared_ptr<buildin_type> bti64 = create_node<buildin_type>(nulltok);
	bti64->value_typecode = buildin_type_code::_sint64;

	boost::shared_ptr<type_definition> tdef0 = create_node<type_definition>(nulltok);
	tdef0->ident = var0_tok;
	tdef0->type_info = bti32;
	boost::shared_ptr<type_definition> tdef1 = create_node<type_definition>(nulltok);
	tdef1->ident = var1_tok;
	tdef1->type_info = bti64;
	boost::shared_ptr<declaration_statement> declstmt0 = create_node<declaration_statement>();
	boost::shared_ptr<declaration_statement> declstmt1 = create_node<declaration_statement>();
	declstmt0->decl = tdef0;
	declstmt1->decl = tdef1;
	prog->decls.push_back( declstmt0 );
	prog->decls.push_back( declstmt1 );

	semantic_analysis( prog, cim );

	boost::shared_ptr<symbol> var0sym = prog->symbol()->find_mangled_this( var_name_0 );
	boost::shared_ptr<symbol> var1sym = prog->symbol()->find_mangled_this( var_name_1 );

	boost::shared_ptr<type_symbol_info> var0tsi = var0sym->symbol_info<type_symbol_info>();
	boost::shared_ptr<type_symbol_info> var1tsi = var1sym->symbol_info<type_symbol_info>();
	
	BOOST_CHECK( var0tsi->type_type() == type_types::buildin );
	BOOST_CHECK( var0tsi->full_type()->value_typecode == buildin_type_code::_sint32 );
	BOOST_CHECK( var1tsi->type_type() == type_types::buildin );
	BOOST_CHECK( var1tsi->full_type()->value_typecode == buildin_type_code::_sint64 );
	
	BOOST_CHECK_EQUAL( cim->all_condition_infos( compiler_informations::none ).size(), 0 );
		
}

BOOST_AUTO_TEST_CASE( name_mangling ){
	using ::sasl::syntax_tree::program;
	using ::sasl::syntax_tree::buildin_type;
	using ::sasl::syntax_tree::create_node;
	using ::sasl::syntax_tree::function_type;
	using ::sasl::syntax_tree::declaration_statement;
	using ::sasl::semantic::semantic_analysis;
	using ::sasl::common::compiler_info_manager;
	using ::sasl::semantic::name_mangler;

	boost::shared_ptr<program> prog = create_node<program>("test");
	
	boost::shared_ptr<buildin_type> bt0 = create_node<buildin_type>( null_token() );
	bt0->value_typecode = buildin_type_code::_sint32;
	boost::shared_ptr<function_type> ft = create_node<function_type>( null_token() );
	ft->is_declaration = true;
	ft->retval_type = new_buildin_type( null_token(), buildin_type_code::_double );
	ft->name = new_token( "foo" );
	boost::shared_ptr<declaration_statement> declstmt = create_node<declaration_statement>( null_token() );
	declstmt->decl = ft;
	prog->decls.push_back( declstmt );

	boost::shared_ptr<compiler_info_manager> cim = compiler_info_manager::create();

	semantic_analysis( prog, cim );
	name_mangler mang;

	BOOST_CHECK_EQUAL( mang.mangle( ft ), std::string("Mfoo@@NNBD@Z") );
}

BOOST_AUTO_TEST_SUITE_END();