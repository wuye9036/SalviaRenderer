#include <boost/test/unit_test.hpp>
#include <sasl/enums/enums_helper.h>
#include <sasl/enums/operators.h>
#include <sasl/include/common/compiler_info_manager.h>
#include <sasl/include/syntax_tree/program.h>
#include <sasl/include/semantic/name_mangler.h>
#include <sasl/include/semantic/semantic_infos.h>
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

BOOST_AUTO_TEST_SUITE( semantic );

BOOST_AUTO_TEST_CASE( program_si_test ){
	semantic_cases::instance();

	BOOST_CHECK( SYNCASE_(prog_for_gen)->symbol() );
	BOOST_CHECK( SYNCASE_(prog_for_gen)->semantic_info() );
	BOOST_CHECK( extract_semantic_info<program_si>(SYNCASE_(prog_for_gen))->name()
		== SYNCASENAME_(prog_for_gen) );
}

BOOST_AUTO_TEST_CASE( function_si_test ){
	semantic_cases::instance();
}

BOOST_AUTO_TEST_CASE( expression_si_test ){
	semantic_cases::instance();

	BOOST_REQUIRE( SEMCASE_(cexpr_776uint) );
	BOOST_CHECK( SEMCASE_(cexpr_776uint)->value<uint32_t>() == 776 );
}

BOOST_AUTO_TEST_CASE( mangling_test ){
	semantic_cases::instance();

	BOOST_CHECK_EQUAL( mangle( SYNCASE_(func_nnn) ), std::string("M") + SYNCASENAME_(func_nnn) + std::string("@@") );
	BOOST_CHECK_EQUAL( mangle( SYNCASE_(func_flt_2p_n_gen) ), std::string("M") + SYNCASENAME_(func_flt_2p_n_gen) + std::string("@@QBU8@@") + std::string("QBS1@@") );

	std::vector<operators> oplist = sasl_ehelper::list_of_operators();
	for( size_t i_op = 0; i_op < oplist.size(); ++i_op ){
		BOOST_CHECK_EQUAL( operator_name( oplist[i_op] ), std::string("0") + oplist[i_op].name() );
	}
}

BOOST_AUTO_TEST_CASE( operators_classifier_test ){
	BOOST_CHECK( sasl_ehelper::is_arithmetic(operators::add) );
	BOOST_CHECK( !sasl_ehelper::is_assign(operators::add) );
	BOOST_CHECK( sasl_ehelper::is_bit_assign( operators::bit_and_assign ) );
	BOOST_CHECK( !sasl_ehelper::is_bool_arith( operators::bit_and_assign ) );

	// Add other tests if any error occurs in regression test.
}

BOOST_AUTO_TEST_CASE( symbol_test ){
	semantic_cases::instance();
	BOOST_REQUIRE( SEMCASE_(sym_root) );
	BOOST_CHECK( SEMCASE_(sym_root)->find_overloads( SYNCASENAME_(func_flt_2p_n_gen) ).size() == 1 );
	BOOST_REQUIRE( SEMCASE_(sym_f0) );
	BOOST_CHECK( SEMCASE_(sym_root)->find( mangle( SYNCASE_(func_flt_2p_n_gen) ) ) == SEMCASE_(sym_f0) );
	BOOST_CHECK( SEMCASE_(sym_f0)->mangled_name() == mangle( SYNCASE_(func_flt_2p_n_gen) ) );
	BOOST_CHECK( SEMCASE_(sym_f0)->unmangled_name() == SYNCASENAME_(func_flt_2p_n_gen) );
	BOOST_CHECK( SEMCASE_(sym_f0)->parent() == SEMCASE_(sym_root) );
	BOOST_CHECK( !SEMCASE_(sym_root)->find( SYNCASENAME_(p0_fn0) ) );
	BOOST_CHECK( SEMCASE_(sym_root)->find_overloads( SYNCASENAME_(p0_fn0) ).empty() );
	BOOST_REQUIRE( SEMCASE_(sym_p0) );
	BOOST_CHECK( SEMCASE_(sym_f0)->find( SYNCASENAME_(p0_fn0) ) == SEMCASE_(sym_p0) );
	BOOST_CHECK( SEMCASE_(sym_f0)->find_overloads( SYNCASENAME_(p0_fn0) ).empty() );
	BOOST_CHECK( SEMCASE_(sym_p0)->find_overloads( SYNCASENAME_(func_flt_2p_n_gen) )[0] == SEMCASE_(sym_f0) );
	BOOST_CHECK( SEMCASE_(sym_p0)->unmangled_name() == SYNCASENAME_(p0_fn0) );
	BOOST_CHECK( SEMCASE_(sym_p0)->mangled_name() == SYNCASENAME_(p0_fn0) );

	std::vector<operators> oplist = sasl_ehelper::list_of_operators();
	BOOST_CHECK( SYNCASE_(prog_for_gen)->symbol()->find_overloads( operator_name(operators::none) ).size() == 0 );

	for( size_t i_op = 0; i_op < oplist.size(); ++i_op ){
		operators op = oplist[i_op];
		std::string opname = operator_name(op);

		const std::vector< boost::shared_ptr<symbol> >& op_in_func = SYNCASE_(prog_for_gen)->symbol()->find_overloads( opname );
		const std::vector< boost::shared_ptr<symbol> >& op_in_global = SYNCASE_(func_flt_2p_n_gen)->symbol()->find_overloads( opname );

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

BOOST_AUTO_TEST_SUITE_END();
