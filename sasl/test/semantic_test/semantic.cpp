#include <boost/test/unit_test.hpp>
#include <sasl/include/common/compiler_info_manager.h>
#include <sasl/include/syntax_tree/program.h>
#include <sasl/include/semantic/semantic_infos.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/test/test_cases/syntax_cases.h>
#include <sasl/test/test_cases/semantic_cases.h>
#include <string>

using ::sasl::common::compiler_info_manager;

using ::sasl::syntax_tree::program;

using ::sasl::semantic::program_si;
using ::sasl::semantic::extract_semantic_info;

#define SYNCASE_(case_name) syntax_cases::instance().##case_name##()
#define SYNCASENAME_( case_name ) syntax_cases::instance().##case_name##_name()
#define SEMCASE_(case_name) semantic_cases::instance().##case_name##()
#define SEMCASENAME_( case_name ) semantic_cases::instance().##case_name##_name()

BOOST_AUTO_TEST_SUITE( semantic );

BOOST_AUTO_TEST_CASE( program_si_test ){
	semantic_cases::instance();

	BOOST_CHECK( SYNCASE_(prog_for_gen)->symbol() );
	BOOST_CHECK( SYNCASE_(prog_for_gen)->semantic_info() );
	BOOST_CHECK( extract_semantic_info<program_si>(SYNCASE_(prog_for_gen))->name()
		== SYNCASENAME_(prog_for_gen) );
}

BOOST_AUTO_TEST_CASE( function_si_test ){
}

BOOST_AUTO_TEST_CASE( expression_si_test ){
	BOOST_CHECK( SEMCASE_(cexpr_776uint) );
	BOOST_CHECK( SEMCASE_(cexpr_776uint)->value<uint32_t>() == 776 );
}

//
//BOOST_AUTO_TEST_CASE( constant_expr_semantic ){
//	using ::sasl::syntax_tree::program;
//	using ::sasl::syntax_tree::declaration_statement;
//	using ::sasl::syntax_tree::variable_declaration;
//	using ::sasl::syntax_tree::constant_expression;
//	using ::sasl::syntax_tree::create_node;
//	using ::sasl::syntax_tree::buildin_type;
//	using ::sasl::syntax_tree::expression_initializer;
//	using ::sasl::semantic::semantic_analysis;
//	using ::sasl::common::token_attr;
//	using ::sasl::semantic::extract_semantic_info;
//	using ::sasl::semantic::const_value_semantic_info;
//	using ::sasl::common::compiler_info_manager;
//
//	using ::sasl::syntax_tree::make_tree;
//
//	std::string prog_name("test");
//	std::string var_name("test_var");
//	
//	std::string var_intval("1.0f");
//
//	boost::shared_ptr<token_attr> nulltok;
//
//	// create
//	boost::shared_ptr<program> prog = create_node<program>( prog_name );
//
//	
//	boost::shared_ptr<buildin_type> vartype = create_node<buildin_type>( nulltok );
//	boost::shared_ptr<token_attr> name_tok( new token_attr(var_name.begin(), var_name.end()) );
//	boost::shared_ptr<constant_expression> initexpr = create_node<constant_expression>( nulltok );
//	boost::shared_ptr<expression_initializer> init = create_node<expression_initializer>( nulltok );
//	boost::shared_ptr<token_attr> intval_tok( new token_attr(var_intval.begin(), var_intval.end()) );
//
//	// assemble
//	initexpr->ctype = literal_constant_types::real;
//	initexpr->value_tok = intval_tok;
//	vartype->value_typecode = buildin_type_code::_sint32;
//	init->init_expr = initexpr;
//
//	boost::shared_ptr<variable_declaration> decl = make_tree( vartype, name_tok, init );
//	prog->d( decl );
//
//	boost::shared_ptr<compiler_info_manager> cim = compiler_info_manager::create();
//
//	// do semantic
//	semantic_analysis( prog, cim );
//
//	// check
//	boost::shared_ptr<const_value_semantic_info> cvsi = extract_semantic_info<const_value_semantic_info>(decl);
//	double val = cvsi->value<double>();
//
//	BOOST_CHECK_EQUAL( (double)1.0f, val );
//	BOOST_CHECK( cvsi->value_type() == buildin_type_code::_float );
//}
//
//BOOST_AUTO_TEST_CASE( type_definition_semantic ){
//	using ::sasl::syntax_tree::program;
//	using ::sasl::syntax_tree::create_node;
//	using ::sasl::syntax_tree::type_definition;
//	using ::sasl::syntax_tree::buildin_type;
//	using ::sasl::syntax_tree::type_specifier;
//	using ::sasl::common::token_attr;
//	using ::sasl::semantic::type_semantic_info;
//	using ::sasl::semantic::semantic_analysis;
//	using ::sasl::syntax_tree::declaration;
//	using ::sasl::syntax_tree::declaration_statement;
//	using ::sasl::semantic::extract_semantic_info;
//	using ::sasl::semantic::symbol;
//	using ::sasl::common::compiler_info_manager;
//
//	boost::shared_ptr<compiler_info_manager> cim = compiler_info_manager::create();
//
//	boost::shared_ptr<token_attr> nulltok;
//
//	std::string var_name_0("var0");
//	std::string var_name_1("var1");
//
//	boost::shared_ptr<token_attr> var0_tok( new token_attr() );
//	var0_tok->str = var_name_0;
//	boost::shared_ptr<token_attr> var1_tok( new token_attr() );
//	var1_tok->str = var_name_1;
//	
//	boost::shared_ptr<program> prog = create_node<program>("test");
//	boost::shared_ptr<buildin_type> bti32 = create_node<buildin_type>(nulltok);
//	bti32->value_typecode = buildin_type_code::_sint32;
//	boost::shared_ptr<buildin_type> bti32_a = create_node<buildin_type>(nulltok);
//	bti32->value_typecode = buildin_type_code::_sint32;
//	boost::shared_ptr<buildin_type> bti64 = create_node<buildin_type>(nulltok);
//	bti64->value_typecode = buildin_type_code::_sint64;
//
//	boost::shared_ptr<type_definition> tdef0 = create_node<type_definition>(nulltok);
//	tdef0->ident = var0_tok;
//	tdef0->type_info = bti32;
//	boost::shared_ptr<type_definition> tdef1 = create_node<type_definition>(nulltok);
//	tdef1->ident = var1_tok;
//	tdef1->type_info = bti64;
//
//	prog->
//		d( tdef0 )
//		.d( tdef1 );
//
//	semantic_analysis( prog, cim );
//
//	boost::shared_ptr<symbol> var0sym = prog->symbol()->find_mangled_this( var_name_0 );
//	boost::shared_ptr<symbol> var1sym = prog->symbol()->find_mangled_this( var_name_1 );
//
//	boost::shared_ptr<type_semantic_info> var0tsi = extract_semantic_info<type_semantic_info>( var0sym->node() );
//	boost::shared_ptr<type_semantic_info> var1tsi = extract_semantic_info<type_semantic_info>( var1sym->node() );
//	
//	BOOST_CHECK( var0tsi->type_type() == type_types::buildin );
//	BOOST_CHECK( var0tsi->full_type()->value_typecode == buildin_type_code::_sint32 );
//	BOOST_CHECK( var1tsi->type_type() == type_types::buildin );
//	BOOST_CHECK( var1tsi->full_type()->value_typecode == buildin_type_code::_sint64 );
//	
//	BOOST_CHECK_EQUAL( cim->all_condition_infos( compiler_informations::none ).size(), 0 );
//		
//}
//
//BOOST_AUTO_TEST_CASE( name_mangling ){
//	using ::sasl::syntax_tree::program;
//	using ::sasl::syntax_tree::buildin_type;
//	using ::sasl::syntax_tree::create_node;
//	using ::sasl::syntax_tree::function_type;
//	using ::sasl::syntax_tree::declaration_statement;
//
//	using ::sasl::syntax_tree::function_tag;
//
//	using ::sasl::semantic::semantic_analysis;
//	using ::sasl::common::compiler_info_manager;
//	using ::sasl::semantic::name_mangler;
//
//	boost::shared_ptr<program> prog = create_node<program>("test");
//	
//	boost::shared_ptr<buildin_type> bt0 = make_tree( buildin_type_code::_sint32 );
//	boost::shared_ptr<function_type> ft = 
//		make_tree( 
//			make_tree( buildin_type_code::_double ),
//			make_tree( "foo" ),
//			function_tag()
//			); // double foo()
//
//	prog->d( ft );
//
//	boost::shared_ptr<compiler_info_manager> cim = compiler_info_manager::create();
//
//	semantic_analysis( prog, cim );
//	name_mangler mang;
//
//	BOOST_CHECK_EQUAL( mang.mangle( ft ), std::string("Mfoo@@NNBD@Z") );
//}

BOOST_AUTO_TEST_SUITE_END();