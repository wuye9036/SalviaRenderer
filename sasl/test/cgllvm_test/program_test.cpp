#include <eflib/include/disable_warnings.h>
#include <llvm/Module.h>
#include <eflib/include/enable_warnings.h>

#include <sasl/include/code_generator/llvm/cgllvm_api.h>
#include <sasl/test/test_cases/cgllvm_cases.h>
#include <sasl/test/test_cases/semantic_cases.h>
#include <sasl/test/test_cases/syntax_cases.h>

#include <boost/test/unit_test.hpp>

using sasl::syntax_tree::node;
using sasl::code_generator::llvm_code;

#define SYNCASE_(case_name) syntax_cases::instance().##case_name##()
#define SYNCASENAME_( case_name ) syntax_cases::instance().##case_name##_name()

#define LLVMCASE_( case_name ) cgllvm_cases::instance().##case_name##()

BOOST_AUTO_TEST_SUITE( main_suite )

BOOST_AUTO_TEST_CASE( module_generation_test ){
	using sasl::syntax_tree::program;
	using sasl::syntax_tree::create_node;

	BOOST_CHECK( !LLVMCASE_(null_root) );
	BOOST_CHECK( LLVMCASE_(root) );
	BOOST_CHECK( SYNCASE_(prog_for_gen)->codegen_ctxt() == LLVMCASE_(root) );
	BOOST_CHECK( LLVMCASE_(root)->module() );
	BOOST_CHECK( LLVMCASE_(root)->module()->getModuleIdentifier() == SYNCASENAME_(prog_for_gen) );
	LLVMCASE_(root)->module()->dump();
}

BOOST_AUTO_TEST_CASE( jit_test ){
	BOOST_CHECK( LLVMCASE_(jit) );
	void* pfunc = LLVMCASE_(jit)->get_function( "foo" );
	LLVMCASE_(root)->module()->dump();
	BOOST_CHECK( pfunc );
	BOOST_CHECK_EQUAL( ((uint32_t(*)())(intptr_t)pfunc)(), 797u ); 
}

BOOST_AUTO_TEST_SUITE_END()
