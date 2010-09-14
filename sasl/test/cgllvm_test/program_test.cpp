#include <sasl/include/code_generator/llvm/cgllvm_api.h>
#include <llvm/Module.h>
#include <sasl/include/syntax_tree/program.h>
#include <boost/test/unit_test.hpp>

using sasl::syntax_tree::node;

BOOST_AUTO_TEST_SUITE( main_suite )

BOOST_AUTO_TEST_CASE( module_generation_test ){
	using sasl::syntax_tree::program;
	using sasl::syntax_tree::create_node;

	boost::shared_ptr<llvm::Module> mod = sasl::code_generator::generate_llvm_code( boost::shared_ptr<node>() );
	BOOST_CHECK( !mod );
	//
	boost::shared_ptr<node> root( create_node<program>("test") );
	mod = sasl::code_generator::generate_llvm_code( root );
	BOOST_CHECK( mod );
	BOOST_CHECK( mod->getModuleIdentifier() == "test" );
}

BOOST_AUTO_TEST_SUITE_END()
