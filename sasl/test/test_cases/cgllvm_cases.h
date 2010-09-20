#ifndef SASL_TEST_TEST_CASES_CGLLVM_CASES_H
#define SASL_TEST_TEST_CASES_CGLLVM_CASES_H

#include <sasl/test/test_cases/utility.h>
#include <sasl/include/common/compiler_info_manager.h>
#include <sasl/include/syntax_tree/program.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/expression.h>
#include <sasl/include/code_generator/llvm/cgllvm_api.h>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

class cgllvm_cases{
public:
	static cgllvm_cases& instance();
	static void release();

	TEST_CASE_SP_VARIABLE( CODEGEN_(llvm_code), root );
	TEST_CASE_SP_VARIABLE( CODEGEN_(llvm_code), null_root );
private:
	cgllvm_cases();
	void initialize();

	static boost::shared_ptr<cgllvm_cases> tcase;
	static boost::mutex mtx;
};

#endif // SASL_TEST_TEST_CASES_cgllvm_cases_H