#include <eflib/include/platform/disable_warnings.h>
#include <llvm/Module.h>
#include <eflib/include/platform/enable_warnings.h>

#include <sasl/include/code_generator/llvm/cgllvm_api.h>
#include <sasl/test/test_cases/cgllvm_cases.h>
#include <sasl/test/test_cases/semantic_cases.h>
#include <sasl/test/test_cases/syntax_cases.h>

#include <boost/test/unit_test.hpp>

using sasl::syntax_tree::node;
using sasl::code_generator::llvm_code;

#define SYNCASE_(case_name) syntax_cases::instance().case_name()
#define SYNCASENAME_( case_name ) syntax_cases::instance().case_name##_name()

#define LLVMCASE_( case_name ) cgllvm_cases::instance().case_name()

BOOST_AUTO_TEST_SUITE( main_suite )

BOOST_AUTO_TEST_CASE( jit_test ){
	cgllvm_cases::instance();

	BOOST_CHECK( LLVMCASE_(jit) );
	void* pfunc = LLVMCASE_(jit)->get_function( "foo" );
	void* pfn4_jit = LLVMCASE_(jit)->get_function(SYNCASENAME_(fn4_jit).c_str());
	LLVMCASE_(root)->module()->dump();
	BOOST_CHECK( pfunc );
	BOOST_CHECK_EQUAL( ((uint32_t(*)())(intptr_t)pfunc)(), 782u );
	// BOOST_CHECK_EQUAL( ((int32_t(*)(int32_t, int32_t))(intptr_t)pfn4_jit)( 4, 5 ), 4 );
	// BOOST_CHECK_EQUAL( ((int32_t(*)(int32_t, int32_t))(intptr_t)pfn4_jit)( 9, 6 ), 6 );
}

BOOST_AUTO_TEST_SUITE_END()
