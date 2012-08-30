#ifndef SASL_TEST_TEST_CASES_CGCASES_H
#define SASL_TEST_TEST_CASES_CGCASES_H

#include <eflib/include/platform/config.h>

#include <sasl/test/test_cases/utility.h>
#include <sasl/include/syntax_tree/program.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/expression.h>
#include <sasl/include/codegen/cg_api.h>
#include <sasl/include/codegen/cg_jit.h>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/disable_warnings.h>
#include <boost/thread.hpp>
#include <eflib/include/platform/enable_warnings.h>

class cg_cases{
public:
	static cg_cases& instance();
	static bool is_avaliable();
	static void release();

	TEST_CASE_SP_VARIABLE( CODEGEN_(cg_jit_engine), jit );

	TEST_CASE_SP_VARIABLE( CODEGEN_(cg_module), root );
private:
	cg_cases();
	void initialize();

	static boost::shared_ptr<cg_cases> tcase;
	static boost::mutex mtx;
};

#endif // SASL_TEST_TEST_CASES_cg_cases_H