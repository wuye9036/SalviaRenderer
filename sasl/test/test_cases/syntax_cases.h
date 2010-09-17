#ifndef SASL_TEST_TEST_CASES_SYNTAX_CASES_H
#define SASL_TEST_TEST_CASES_SYNTAX_CASES_H

#include <sasl/test/test_cases/utility.h>
#include <sasl/include/syntax_tree/program.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

class syntax_cases{
public:
	static syntax_cases& instance();
	static void release();
	
	TEST_CASE_SP_VARIABLE( SYNTAX_(program), empty_prog );
	TEST_CASE_CREF_VARIABLE( std::string, prog_name );

	// buildin types
	TEST_CASE_SP_VARIABLE( SYNTAX_(buildin_type), type_sint8 );
	TEST_CASE_SP_VARIABLE( SYNTAX_(buildin_type), type_uint64 );
	TEST_CASE_SP_VARIABLE( SYNTAX_(buildin_type), type_double );
	TEST_CASE_SP_VARIABLE( SYNTAX_(buildin_type), type_boolean );
	TEST_CASE_SP_VARIABLE( SYNTAX_(buildin_type), type_void );
private:
	syntax_cases();
	void initialize();
	static boost::shared_ptr<syntax_cases> tcase;
	static boost::mutex mtx;
};

#endif // SASL_TEST_TEST_CASES_PROGRAM_CASES_H