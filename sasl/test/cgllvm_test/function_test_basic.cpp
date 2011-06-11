#include <eflib/include/platform/disable_warnings.h>
#include <llvm/DerivedTypes.h>
#include <llvm/Module.h>
#include <llvm/Function.h>
#include <eflib/include/platform/enable_warnings.h>
#include <sasl/test/test_cases/cgllvm_cases.h>
#include <sasl/test/test_cases/semantic_cases.h>
#include <sasl/test/test_cases/syntax_cases.h>
#include <boost/test/unit_test.hpp>

#define SYNCASE_(case_name) syntax_cases::instance().case_name ()
#define SYNCASENAME_( case_name ) syntax_cases::instance().case_name##_name()

#define LLVMCASE_( case_name ) cgllvm_cases::instance().case_name ()

BOOST_AUTO_TEST_SUITE( main_suite )

BOOST_AUTO_TEST_CASE( function_param_test ){

}
BOOST_AUTO_TEST_SUITE_END()
