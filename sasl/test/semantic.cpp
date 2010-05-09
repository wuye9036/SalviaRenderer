#include <boost/test/unit_test.hpp>
#include <sasl/include/semantic/semantic_analyser.h>
#include <sasl/include/syntax_tree/node_creation.h>
#include <sasl/include/syntax_tree/program.h>
#include <string>

BOOST_AUTO_TEST_SUITE( semantic );

BOOST_AUTO_TEST_CASE( program_semantic ){
	using ::sasl::syntax_tree::program;
	using ::sasl::syntax_tree::create_node;
	using ::sasl::semantic::semantic_analysis;

	std::string prog_name("test");
	boost::shared_ptr<program> prog = create_node<program>( prog_name );
	BOOST_CHECK( !prog->symbol() );

	semantic_analysis( prog );
	BOOST_CHECK( prog->symbol() );
}

BOOST_AUTO_TEST_SUITE_END();