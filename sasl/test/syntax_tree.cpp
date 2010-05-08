#include <sasl/include/syntax_tree/program.h>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE( program );

BOOST_AUTO_TEST_CASE( create_program ){
	using ::sasl::syntax_tree::create_node;
	using ::sasl::syntax_tree::program;

	create_node<program>( "test" );
}

BOOST_AUTO_TEST_SUITE_END();