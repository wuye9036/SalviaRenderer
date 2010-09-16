#include <sasl/include/code_generator/llvm/cgllvm_api.h>

#include <eflib/include/disable_warnings.h>
#include <llvm/Module.h>
#include <eflib/include/enable_warnings.h>

#include <sasl/include/syntax_tree/program.h>
#include <boost/test/unit_test.hpp>

using sasl::syntax_tree::node;
using sasl::code_generator::llvm_code;

BOOST_AUTO_TEST_SUITE( main_suite )

BOOST_AUTO_TEST_CASE( module_generation_test ){
	using sasl::syntax_tree::program;
	using sasl::syntax_tree::create_node;

	boost::shared_ptr<llvm_code> mod = sasl::code_generator::generate_llvm_code( boost::shared_ptr<node>() );
	BOOST_CHECK( !mod );
	//
	boost::shared_ptr<node> root( create_node<program>("test") );
	mod = sasl::code_generator::generate_llvm_code( root );
	BOOST_CHECK( mod );
	BOOST_CHECK( root->codegen_ctxt() == mod );
	BOOST_CHECK( mod->module() );
	BOOST_CHECK( mod->module()->getModuleIdentifier() == "test" );
}

BOOST_AUTO_TEST_SUITE_END()
