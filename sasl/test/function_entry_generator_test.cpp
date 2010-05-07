#include <sasl/include/code_generator/llvm/cg_llvm.h>
#include <sasl/include/syntax_tree/program.h>

#define BOOST_TEST_MODULE Code Generator Test
#include <boost/test/unit_test.hpp>

using sasl::syntax_tree::node;

struct console_waiter{
	console_waiter(){}
	~console_waiter(){
		system("pause");
	}
};

BOOST_AUTO_TEST_SUITE( main_suite )

BOOST_AUTO_TEST_CASE( generate_null_module ){
	boost::shared_ptr<llvm::Module> mod = sasl::code_generator::generate_llvm_code( boost::shared_ptr<node>() );
	BOOST_CHECK( !mod );
}

BOOST_AUTO_TEST_CASE( generate_empty_module ){
	using sasl::syntax_tree::program;

	boost::shared_ptr<node> root( new program() );
	boost::shared_ptr<llvm::Module> mod = sasl::code_generator::generate_llvm_code( root );
	BOOST_CHECK( mod );
}

BOOST_AUTO_TEST_SUITE_END()
