#include <sasl/include/code_generator/llvm/cg_llvm.h>

#include <sasl/include/code_generator/llvm/llvm_patch_begin.h>
#include <llvm/Module.h>
#include <sasl/include/code_generator/llvm/llvm_patch_end.h>

#include <sasl/include/syntax_tree/program.h>
#include <boost/test/unit_test.hpp>

using sasl::syntax_tree::node;

BOOST_AUTO_TEST_SUITE( main_suite )

BOOST_AUTO_TEST_CASE( module_generation_test ){
	using sasl::syntax_tree::program;

	boost::shared_ptr<llvm::Module> mod = sasl::code_generator::generate_llvm_code( boost::shared_ptr<node>() );
	BOOST_CHECK( !mod );
	//
	boost::shared_ptr<node> root( new program("test") );
	mod = sasl::code_generator::generate_llvm_code( root );
	BOOST_CHECK( mod );
	BOOST_CHECK( mod->getModuleIdentifier() == "test" );
}

BOOST_AUTO_TEST_SUITE_END()
