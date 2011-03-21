#ifndef SASL_TEST_TEST_CASES_SYNTAX_CASES_H
#define SASL_TEST_TEST_CASES_SYNTAX_CASES_H

#include <eflib/include/platform/config.h>

#include <sasl/test/test_cases/utility.h>
#include <sasl/include/syntax_tree/program.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/expression.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <eflib/include/platform/boost_end.h>

class syntax_cases{
public:
	static syntax_cases& instance();
	static bool is_avaliable();
	static void release();

	TEST_CASE_SP_VARIABLE( SYNTAX_(program), empty_prog );
	TEST_CASE_CREF_VARIABLE( std::string, prog_name );
	TEST_CASE_CREF_VARIABLE( float, val_3p25f );
	TEST_CASE_CREF_VARIABLE( float, val_3p08f );
	TEST_CASE_CREF_VARIABLE( float, val_17ushort );
	TEST_CASE_CREF_VARIABLE( uint32_t, val_776uint );
	TEST_CASE_CREF_VARIABLE( uint32_t, val_874uint );
	TEST_CASE_CREF_VARIABLE( uint32_t, val_21uint );

	// builtin type codes.
	TEST_CASE_CREF_VARIABLE( builtin_type_code, btc_sint8 );
	TEST_CASE_CREF_VARIABLE( builtin_type_code, btc_sint32 );
	TEST_CASE_CREF_VARIABLE( builtin_type_code, btc_uint32 );
	TEST_CASE_CREF_VARIABLE( builtin_type_code, btc_uint64 );
	TEST_CASE_CREF_VARIABLE( builtin_type_code, btc_float );
	TEST_CASE_CREF_VARIABLE( builtin_type_code, btc_double );
	TEST_CASE_CREF_VARIABLE( builtin_type_code, btc_boolean );
	TEST_CASE_CREF_VARIABLE( builtin_type_code, btc_void );
	TEST_CASE_CREF_VARIABLE( builtin_type_code, btc_short2 );
	TEST_CASE_CREF_VARIABLE( builtin_type_code, btc_float3 );
	TEST_CASE_CREF_VARIABLE( builtin_type_code, btc_double2x4 );
	TEST_CASE_CREF_VARIABLE( builtin_type_code, btc_ulong3x2 );
	TEST_CASE_CREF_VARIABLE( builtin_type_code, btc_none );

	TEST_CASE_SP_VARIABLE( SYNTAX_(constant_expression), cexpr_3p25f );
	TEST_CASE_SP_VARIABLE( SYNTAX_(constant_expression), cexpr_17ushort );
	TEST_CASE_SP_VARIABLE( SYNTAX_(constant_expression), cexpr_776uint );
	TEST_CASE_SP_VARIABLE( SYNTAX_(constant_expression), cexpr_874uint );
	TEST_CASE_SP_VARIABLE( SYNTAX_(constant_expression), cexpr_21uint );
	TEST_CASE_SP_VARIABLE( SYNTAX_(expression_initializer), exprinit_cexpr_3p25f );

	// builtin types
	TEST_CASE_SP_VARIABLE( SYNTAX_(builtin_type), type_sint8 );
	TEST_CASE_SP_VARIABLE( SYNTAX_(builtin_type), type_sint32 );
	TEST_CASE_SP_VARIABLE( SYNTAX_(builtin_type), type_uint32 );
	TEST_CASE_SP_VARIABLE( SYNTAX_(builtin_type), type_uint64 );
	TEST_CASE_SP_VARIABLE( SYNTAX_(builtin_type), type_float );
	TEST_CASE_SP_VARIABLE( SYNTAX_(builtin_type), type_double );
	TEST_CASE_SP_VARIABLE( SYNTAX_(builtin_type), type_boolean );
	TEST_CASE_SP_VARIABLE( SYNTAX_(builtin_type), type_void );
	TEST_CASE_SP_VARIABLE( SYNTAX_(builtin_type), type_short2 );
	TEST_CASE_SP_VARIABLE( SYNTAX_(builtin_type), type_float3 );
	TEST_CASE_SP_VARIABLE( SYNTAX_(builtin_type), type_double2x4 );
	TEST_CASE_SP_VARIABLE( SYNTAX_(builtin_type), type_ulong3x2 );

	// variables
	TEST_CASE_SP_VARIABLE( SYNTAX_(variable_declaration), var_int8 );
	TEST_CASE_SP_VARIABLE( SYNTAX_(variable_declaration), var_float_3p25f );
	TEST_CASE_SP_VARIABLE( SYNTAX_(variable_declaration), var_3_float );
	TEST_CASE_SP_VARIABLE( SYNTAX_(declarator), var_3_0 );
	TEST_CASE_SP_VARIABLE( SYNTAX_(declarator), var_3_1_with_init );
	TEST_CASE_SP_VARIABLE( SYNTAX_(declarator), var_3_2_with_nullinit );

	TEST_CASE_SP_VARIABLE( SYNTAX_(type_definition), tdef0_double2x4 );

	// functions
	TEST_CASE_SP_VARIABLE( SYNTAX_(function_type), fn0_sem );
	TEST_CASE_SP_VARIABLE( SYNTAX_(function_type), fn1_sem );
	TEST_CASE_SP_VARIABLE( SYNTAX_(function_type), fn2_sem );
	TEST_CASE_SP_VARIABLE( SYNTAX_(function_type), fn3_jit );
	TEST_CASE_SP_VARIABLE( SYNTAX_(function_type), fn4_jit ); // Test for if-statement

	// paramters
	TEST_CASE_SP_VARIABLE( SYNTAX_(parameter), par0_0_fn1 );
	TEST_CASE_SP_VARIABLE( SYNTAX_(parameter), par1_1_fn1 );

	// expressions
	TEST_CASE_SP_VARIABLE( SYNTAX_(expression), expr0_add );
	TEST_CASE_SP_VARIABLE( SYNTAX_(expression), expr1_add );

	// statements
	TEST_CASE_SP_VARIABLE( SYNTAX_(compound_statement), fn1_body );

	// prog
	TEST_CASE_SP_VARIABLE( SYNTAX_(program), null_prog );
	TEST_CASE_SP_VARIABLE( SYNTAX_(program), prog_for_syntax_test );
	TEST_CASE_SP_VARIABLE( SYNTAX_(program), prog_for_semantic_test );
	TEST_CASE_SP_VARIABLE( SYNTAX_(program), prog_for_jit_test );
private:
	syntax_cases();
	void initialize();
	static boost::shared_ptr<syntax_cases> tcase;
	static boost::mutex mtx;
};

#endif // SASL_TEST_TEST_CASES_PROGRAM_CASES_H
