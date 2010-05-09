#include <boost/test/unit_test.hpp>
#include <sasl/include/semantic/semantic_analyser.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/semantic/symbol_infos.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/expression.h>
#include <sasl/include/syntax_tree/node_creation.h>
#include <sasl/include/syntax_tree/program.h>
#include <sasl/include/syntax_tree/statement.h>
#include <sasl/include/common/token_attr.h>
#include <string>

BOOST_AUTO_TEST_SUITE( semantic );

BOOST_AUTO_TEST_CASE( program_semantic ){
	using ::sasl::syntax_tree::program;
	using ::sasl::syntax_tree::create_node;
	using ::sasl::semantic::semantic_analysis;

	std::string prog_name("test");
	boost::shared_ptr<program> prog = create_node<program>( prog_name );
	BOOST_CHECK( !prog->symbol() );

	semantic_analysis( prog );
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

	std::string prog_name("test");
	std::string var_name("test_var");
	
	std::string var_intval("9987");

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
	initexpr->ctype = literal_constant_types::integer;
	initexpr->value_tok = intval_tok;
	vartype->value_typecode = buildin_type_code::_sint32;
	init->init_expr = initexpr;
	decl->init = init;
	decl->name = name_tok;
	decl->type_info = vartype;
	declstmt->decl = decl;
	prog->decls.push_back( declstmt );

	// do semantic
	semantic_analysis( prog );

	// check
	boost::shared_ptr<const_value_symbol_info> cvsi = extract_symbol_info<const_value_symbol_info>(decl);
	long val = cvsi->value<long>();

	BOOST_CHECK_EQUAL( (long)9987, val );
	BOOST_CHECK( cvsi->value_type() == buildin_type_code::_sint32 );
}

BOOST_AUTO_TEST_SUITE_END();