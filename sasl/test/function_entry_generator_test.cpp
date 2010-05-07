#include <sasl/include/code_generator/llvm/cg_llvm.h>

#define BOOST_TEST_MODULE Code Generator Test
#include <boost/test/unit_test.hpp>

using sasl::syntax_tree::node;

struct console_waiter{
	console_waiter(){}
	~console_waiter(){
		system("pause");
	}
};

BOOST_FIXTURE_TEST_SUITE( main_suite, console_waiter )

BOOST_AUTO_TEST_CASE( generate_module ){
	boost::shared_ptr<llvm::Module> mod = sasl::code_generator::generate_llvm_code( boost::shared_ptr<node>() );
	BOOST_CHECK( !mod );
}

BOOST_AUTO_TEST_SUITE_END()
