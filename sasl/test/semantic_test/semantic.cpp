#include <eflib/include/platform/boost_begin.h>
#include <boost/test/unit_test.hpp>
#include <eflib/include/platform/boost_end.h>

#include <sasl/enums/enums_helper.h>
#include <sasl/enums/operators.h>
#include <sasl/include/common/compiler_info_manager.h>
#include <sasl/include/syntax_tree/program.h>
#include <sasl/include/semantic/name_mangler.h>
#include <sasl/include/semantic/semantic_infos.imp.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/test/test_cases/syntax_cases.h>
#include <sasl/test/test_cases/semantic_cases.h>
#include <string>

using ::sasl::common::compiler_info_manager;

using ::sasl::syntax_tree::program;

using ::sasl::semantic::extract_semantic_info;
using ::sasl::semantic::mangle;
using ::sasl::semantic::operator_name;
using ::sasl::semantic::program_si;
using ::sasl::semantic::symbol;

#define SYNCASE_(case_name) syntax_cases::instance().case_name()
#define SYNCASENAME_( case_name ) syntax_cases::instance().case_name##_name()
#define SEMCASE_(case_name) semantic_cases::instance().case_name()
#define SEMCASENAME_( case_name ) semantic_cases::instance().case_name##_name()

void test_global_si(){
	semantic_cases::instance();
	BOOST_REQUIRE( SEMCASE_( si_root ) );
}

void test_program_si(){
	semantic_cases::instance();

	BOOST_REQUIRE( SEMCASE_(sym_root) );
}

void test_function_si(){
	semantic_cases::instance();
	BOOST_REQUIRE( SEMCASE_(sym_fn0_sem) );
	BOOST_REQUIRE( SEMCASE_(sym_fn1_sem) );
}

void test_statement_si(){
	semantic_cases::instance();

	BOOST_REQUIRE( SEMCASE_(sym_fn1_sem) );
	BOOST_REQUIRE( SEMCASE_(fn1_sem)->body );
	BOOST_CHECK( SEMCASE_(fn1_sem)->body->symbol()->parent() =  SEMCASE_(sym_fn1_sem) );
	BOOST_REQUIRE( SEMCASE_(sym_fn2_sem) );
	BOOST_REQUIRE( SEMCASE_(body_fn2) );
	BOOST_REQUIRE( SEMCASE_(sym_body_fn2) );
	BOOST_CHECK( SEMCASE_(sym_body_fn2)->parent() != SEMCASE_(sym_root) );
	BOOST_CHECK( SEMCASE_(sym_body_fn2)->parent() == SEMCASE_(sym_fn2_sem) );
	BOOST_CHECK_EQUAL( SEMCASE_(body_fn2)->stmts.size(), 1 );
	BOOST_REQUIRE( SEMCASE_(jstmt0_0_fn2) );
}

void test_expression_si(){
	semantic_cases::instance();

	BOOST_REQUIRE( SEMCASE_(bexpr0_0_jstmts0) );
	BOOST_REQUIRE( SEMCASE_(cexpr0_l_bexpr0) );
	BOOST_REQUIRE( SEMCASE_(si_cexpr0) );
	BOOST_CHECK( SEMCASE_(op_bexpr0) == operators::add );
	BOOST_CHECK( SEMCASE_(si_cexpr0)->value<uint32_t>() == 776 );
	BOOST_CHECK( SEMCASE_(si_cexpr0)->value_type() == buildin_type_code::_uint32 );
}

void test_mangle(){
	semantic_cases::instance();
	BOOST_CHECK_EQUAL( SEMCASE_(sym_fn0_sem)->mangled_name(), SEMCASE_(mangled_fn0_name) );
	BOOST_CHECK_EQUAL( SEMCASE_(sym_fn1_sem)->mangled_name(), SEMCASE_(mangled_fn1_name) );

	std::vector<operators> oplist = sasl_ehelper::list_of_operators();
	for( size_t i_op = 0; i_op < oplist.size(); ++i_op ){
		BOOST_CHECK_EQUAL( operator_name( oplist[i_op] ), std::string("0") + oplist[i_op].name() );
	}
}

void test_unmangle(){
	semantic_cases::instance();
	BOOST_CHECK_EQUAL( SEMCASE_(sym_fn0_sem)->unmangled_name(), SYNCASENAME_(fn0_sem) );
}

void test_symbol(){
	semantic_cases::instance();

	// function in module
	BOOST_CHECK_EQUAL( SEMCASE_(sym_root)->find_overloads( SYNCASENAME_(fn1_sem) ).size(), 1 );
	BOOST_CHECK( SEMCASE_(sym_root)->find( SEMCASE_(mangled_fn0_name) ) == SEMCASE_(sym_fn0_sem) );

	// symbol and node
	BOOST_CHECK( SEMCASE_(fn0_sem)->symbol() == SEMCASE_(sym_fn0_sem) );
	
	// parent and sibling
	BOOST_CHECK( SEMCASE_(sym_fn0_sem)->parent() == SEMCASE_(sym_root) );
	BOOST_CHECK( SEMCASE_(sym_fn1_sem)->find( SEMCASE_(mangled_fn0_name) ) == SEMCASE_(sym_fn0_sem) );

	// parameter in function
	BOOST_REQUIRE( SEMCASE_(sym_par0_0_fn1) );
	BOOST_REQUIRE( SEMCASE_(par1_1_fn1) );

	BOOST_CHECK( SEMCASE_(fn1_sem)->params[0] == SEMCASE_(par0_0_fn1) );
	BOOST_CHECK( SEMCASE_(fn1_sem)->params[1] == SEMCASE_(par1_1_fn1) );

	BOOST_CHECK_EQUAL( SEMCASE_(sym_par0_0_fn1)->mangled_name(), SYNCASENAME_(par0_0_fn1) );
	BOOST_CHECK_EQUAL( SEMCASE_(sym_par0_0_fn1)->unmangled_name(), SYNCASENAME_(par0_0_fn1) );
}

void test_default_operators(){
	semantic_cases::instance();

	// Test operator classification functions.
	BOOST_CHECK( sasl_ehelper::is_arithmetic(operators::add) );
	BOOST_CHECK( !sasl_ehelper::is_assign(operators::add) );

	BOOST_CHECK( sasl_ehelper::is_bit_assign( operators::bit_and_assign ) );
	BOOST_CHECK( !sasl_ehelper::is_bool_arith( operators::bit_and_assign ) );

	std::vector<operators> oplist = sasl_ehelper::list_of_operators();
	BOOST_CHECK_EQUAL( SEMCASE_(sym_root)->find_overloads( operator_name(operators::none) ).size(), 0 );

	// Test could operators be accessed globally.
	for( size_t i_op = 0; i_op < oplist.size(); ++i_op ){
		operators op = oplist[i_op];
		std::string opname = operator_name(op);

		const std::vector< boost::shared_ptr<symbol> >& op_in_func = SEMCASE_(sym_root)->find_overloads( opname );
		const std::vector< boost::shared_ptr<symbol> >& op_in_global = SEMCASE_(sym_fn1_sem)->find_overloads( opname );

		if ( oplist[i_op] == operators::none ){
			BOOST_CHECK( op_in_global.empty() );
		} else {
			BOOST_CHECK( !op_in_global.empty() );
			if ( op_in_global.empty() ){
				BOOST_TEST_MESSAGE( opname );
			}
		}

		BOOST_CHECK_EQUAL_COLLECTIONS( op_in_global.begin(), op_in_global.end(), op_in_func.begin(), op_in_func.end() );
	}

}

BOOST_AUTO_TEST_SUITE( semantic );

BOOST_AUTO_TEST_CASE( semantic_tests ){
	test_global_si();
	test_program_si();
	test_function_si();
	test_mangle();
	test_unmangle();
	test_symbol();
	test_statement_si();
	test_expression_si();
	test_default_operators();
}

BOOST_AUTO_TEST_SUITE_END();
