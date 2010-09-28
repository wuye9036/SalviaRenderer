#ifndef SASL_TEST_TEST_CASES_SEMANTIC_CASES_H
#define SASL_TEST_TEST_CASES_SEMANTIC_CASES_H

#include <eflib/include/config.h>

#include <sasl/test/test_cases/utility.h>
#include <sasl/include/common/compiler_info_manager.h>
#include <sasl/include/syntax_tree/program.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/expression.h>
#include <sasl/include/semantic/semantic_infos.h>
#include <boost/shared_ptr.hpp>
#include <eflib/include/disable_warnings.h>
#include <boost/thread.hpp>
#include <eflib/include/enable_warnings.h>

class semantic_cases{
public:
	static semantic_cases& instance();
	static void release();
	TEST_CASE_SP_VARIABLE( SEMANTIC_(const_value_si), cexpr_776uint );
	TEST_CASE_SP_VARIABLE( SEMANTIC_(symbol), sym_root )
	TEST_CASE_SP_VARIABLE( SEMANTIC_(symbol), sym_f0 );
	TEST_CASE_SP_VARIABLE( SEMANTIC_(symbol), sym_p0 );
	TEST_CASE_SP_VARIABLE( SEMANTIC_(symbol), sym_p1 );
private:
	semantic_cases();
	void initialize();

	boost::shared_ptr<::sasl::common::compiler_info_manager> cim;

	static boost::shared_ptr<semantic_cases> tcase;
	static boost::mutex mtx;
};

#endif // SASL_TEST_TEST_CASES_SEMANTIC_CASES_H