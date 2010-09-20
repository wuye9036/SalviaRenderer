#include <eflib/include/disable_warnings.h>
#include <llvm/DerivedTypes.h>
#include <llvm/Module.h>
#include <llvm/Function.h>
#include <eflib/include/enable_warnings.h>
#include <sasl/test/test_cases/cgllvm_cases.h>
#include <sasl/test/test_cases/syntax_cases.h>
#include <boost/test/unit_test.hpp>

#define SYNCASE_(case_name) syntax_cases::instance().##case_name##()
#define SYNCASENAME_( case_name ) syntax_cases::instance().##case_name##_name()

#define LLVMCASE_( case_name ) cgllvm_cases::instance().##case_name##()

BOOST_AUTO_TEST_SUITE( main_suite )

BOOST_AUTO_TEST_CASE( function_generation_test ){
	llvm::Function* func = LLVMCASE_(root)->module()->getFunction( SYNCASENAME_(func_nnn) );
	BOOST_CHECK( func );
	BOOST_CHECK( func->getFunctionType()->getReturnType() );
	BOOST_CHECK( func->getFunctionType()->getReturnType()->isVoidTy() );
}

BOOST_AUTO_TEST_SUITE_END()
