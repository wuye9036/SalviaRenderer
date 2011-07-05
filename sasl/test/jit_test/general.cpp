#include <eflib/include/platform/boost_begin.h>
#include <boost/test/unit_test.hpp>
#include <eflib/include/platform/boost_end.h>

#include <sasl/include/compiler/options.h>
#include <sasl/include/code_generator/llvm/cgllvm_api.h>
#include <sasl/include/code_generator/llvm/cgllvm_jit.h>

using sasl::compiler::compiler;

using sasl::code_generator::jit_engine;
using sasl::code_generator::cgllvm_jit_engine;
using sasl::code_generator::llvm_module;

using boost::shared_ptr;
using boost::shared_polymorphic_cast;

using std::string;

BOOST_AUTO_TEST_SUITE( jit )

string make_command( string const& file_name, string const& options = "--lang=g"){
	return "--input=\"" + file_name + "\" " + options;
}


struct jit_fixture {
	
	jit_fixture() {
		c.parse( make_command( "./repo/question/v1a1/preprocessors.ss" ) );

		bool aborted = false;
		c.process( aborted );

		BOOST_REQUIRE( c.root() );
		BOOST_REQUIRE( c.module_sem() );
		BOOST_REQUIRE( c.module_codegen() );

		std::string jit_err;

		je = cgllvm_jit_engine::create( shared_polymorphic_cast<llvm_module>(c.module_codegen()), jit_err );
		BOOST_REQUIRE( je );
	}

	~jit_fixture(){}

	compiler c;
	shared_ptr<jit_engine> je;
};

BOOST_FIXTURE_TEST_CASE( preprocessors, jit_fixture ){
	int(*p)() = static_cast<int(*)()>( je->get_function("Mmain@@") );
	BOOST_REQUIRE(p);

	BOOST_CHECK( p() == 0 );
}

BOOST_AUTO_TEST_SUITE_END();