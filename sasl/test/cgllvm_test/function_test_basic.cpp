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

BOOST_AUTO_TEST_CASE( function_generation_test ){
	cgllvm_cases::instance();

	llvm::Function* func = LLVMCASE_(root)->module()->getFunction( SYNCASENAME_(fn0_sem) );
	BOOST_CHECK( func );
	BOOST_CHECK( func->getFunctionType()->getReturnType() );
	BOOST_CHECK( func->getFunctionType()->getReturnType()->isVoidTy() );
	BOOST_CHECK( LLVMCASE_(root)->module()->getFunction( SYNCASENAME_(fn1_sem) ) );
	BOOST_CHECK( LLVMCASE_(fn1_sem) );
	BOOST_CHECK( LLVMCASE_(fn1_sem_p1) );
	BOOST_CHECK( !LLVMCASE_(fn1_sem_p0)->is_signed );
	BOOST_CHECK( LLVMCASE_(fn1_sem_p1)->is_signed );
	BOOST_CHECK( LLVMCASE_(fn1_sem)->func_type );
	BOOST_CHECK( LLVMCASE_(fn1_sem)->func_type->getParamType(0)->isIntegerTy() );
	BOOST_CHECK( LLVMCASE_(fn1_sem)->func_type->getReturnType()->isFloatTy() );
	BOOST_CHECK( LLVMCASE_(fn1_sem)->func );
	BOOST_CHECK( !LLVMCASE_(fn1_sem)->func->isVarArg() );
	BOOST_CHECK( LLVMCASE_(fn1_sem)->func->arg_begin()->getName() == SYNCASENAME_(par0_0_fn1) );
}

BOOST_AUTO_TEST_CASE( function_param_test ){

}
BOOST_AUTO_TEST_SUITE_END()
