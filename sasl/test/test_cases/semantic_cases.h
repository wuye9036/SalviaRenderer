#ifndef SASL_TEST_TEST_CASES_SEMANTIC_CASES_H
#define SASL_TEST_TEST_CASES_SEMANTIC_CASES_H

#include <sasl/test/test_cases/utility.h>
#include <sasl/include/common/compiler_info_manager.h>
#include <sasl/include/syntax_tree/program.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/expression.h>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

class semantic_cases{
public:
	static semantic_cases& instance();
	static void release();
private:
	semantic_cases();
	void initialize();

	boost::shared_ptr<::sasl::common::compiler_info_manager> cim;

	static boost::shared_ptr<semantic_cases> tcase;
	static boost::mutex mtx;
};

#endif // SASL_TEST_TEST_CASES_SEMANTIC_CASES_H