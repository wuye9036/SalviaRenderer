#ifndef SASL_TEST_TEST_CASES_SEMANTIC_CASES_H
#define SASL_TEST_TEST_CASES_SEMANTIC_CASES_H

#include <eflib/include/platform/config.h>

#include <sasl/test/test_cases/utility.h>
#include <sasl/enums/operators.h>
#include <sasl/include/syntax_tree/program.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/expression.h>
#include <sasl/include/syntax_tree/statement.h>
#include <sasl/include/semantic/semantic_infos.h>
#include <sasl/include/semantic/semantics.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <eflib/include/platform/boost_end.h>

class semantic_cases{
public:
	static semantic_cases& instance();
	static bool is_avaliable();
	static void release();

	TEST_CASE_SP_VARIABLE( SEMANTIC_(module_semantic), si_root );
	TEST_CASE_SP_VARIABLE( SEMANTIC_(symbol), sym_root );

	TEST_CASE_SP_VARIABLE( SEMANTIC_(symbol), sym_fn0_sem );
	TEST_CASE_CREF_VARIABLE( std::string, mangled_fn0_name );
	TEST_CASE_CREF_VARIABLE( std::string, mangled_fn1_name );
	TEST_CASE_SP_VARIABLE( SYNTAX_(node), fn0_sem );
	TEST_CASE_SP_VARIABLE( SYNTAX_(function_type), fn1_sem );
	TEST_CASE_SP_VARIABLE( SEMANTIC_(type_info_si), si_fn0_sem );
	TEST_CASE_SP_VARIABLE( SEMANTIC_(symbol), sym_fn2_sem );
	TEST_CASE_SP_VARIABLE( SYNTAX_(function_type), fn2_sem );
	TEST_CASE_SP_VARIABLE( SYNTAX_(compound_statement), body_fn2 );
	TEST_CASE_SP_VARIABLE( SEMANTIC_(symbol), sym_body_fn2 );

	TEST_CASE_SP_VARIABLE( SEMANTIC_(symbol), sym_fn1_sem );
	TEST_CASE_SP_VARIABLE( SEMANTIC_(type_info_si), si_fn1_sem );
	TEST_CASE_SP_VARIABLE( SYNTAX_(parameter), par0_0_fn1 );
	TEST_CASE_SP_VARIABLE( SEMANTIC_(symbol), sym_par0_0_fn1 );
	TEST_CASE_SP_VARIABLE( SYNTAX_(parameter), par1_1_fn1 );
	TEST_CASE_SP_VARIABLE( SEMANTIC_(symbol), sym_par1_1_fn1 );

	TEST_CASE_SP_VARIABLE( SYNTAX_(jump_statement), jstmt0_0_fn2 );
	TEST_CASE_SP_VARIABLE( SYNTAX_(binary_expression), bexpr0_0_jstmts0 );
	TEST_CASE_SP_VARIABLE( SYNTAX_(constant_expression), cexpr0_l_bexpr0 );
	TEST_CASE_SP_VARIABLE( SEMANTIC_(const_value_si), si_cexpr0 );
	TEST_CASE_CREF_VARIABLE( operators, op_bexpr0 );
private:
	semantic_cases();
	void initialize();

	boost::shared_ptr<sasl::common::diag_chat> diags;

	static boost::shared_ptr<semantic_cases> tcase;
	static boost::mutex mtx;
};

#endif // SASL_TEST_TEST_CASES_SEMANTIC_CASES_H
