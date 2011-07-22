#include <eflib/include/platform/boost_begin.h>
#include <boost/test/unit_test.hpp>
#include <eflib/include/platform/boost_end.h>

#include <sasl/include/compiler/options.h>
#include <sasl/include/code_generator/llvm/cgllvm_api.h>
#include <sasl/include/code_generator/llvm/cgllvm_jit.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/semantic/semantic_infos.h>

#include <eflib/include/math/vector.h>

#include <fstream>

using sasl::compiler::compiler;

using sasl::semantic::symbol;

using sasl::code_generator::jit_engine;
using sasl::code_generator::cgllvm_jit_engine;
using sasl::code_generator::llvm_module;

using boost::shared_ptr;
using boost::shared_polymorphic_cast;

using std::fstream;
using std::string;

BOOST_AUTO_TEST_SUITE( jit )

string make_command( string const& file_name, string const& options = "--lang=g"){
	return "--input=\"" + file_name + "\" " + options;
}

struct jit_fixture {
	
	jit_fixture() {}

	void init( string const& file_name ){
		c.parse( make_command(file_name) );

		bool aborted = false;
		c.process( aborted );

		BOOST_REQUIRE( c.root() );
		BOOST_REQUIRE( c.module_sem() );
		BOOST_REQUIRE( c.module_codegen() );

		root_sym = c.module_sem()->root();

		fstream dump_file( "jit_test.ll", std::ios::out );
		dump( shared_polymorphic_cast<llvm_module>(c.module_codegen()), dump_file );
		dump_file.close();

		std::string jit_err;

		je = cgllvm_jit_engine::create( shared_polymorphic_cast<llvm_module>(c.module_codegen()), jit_err );
		BOOST_REQUIRE( je );
	}

	template <typename FunctionT>
	void function( FunctionT& fn, string const& unmangled_name ){
		string fn_name = root_sym->find_overloads(unmangled_name)[0]->mangled_name();
		fn = static_cast<FunctionT>( je->get_function(fn_name) );
	}

	~jit_fixture(){}

	compiler c;
	shared_ptr<symbol> root_sym;
	shared_ptr<jit_engine> je;
};

BOOST_FIXTURE_TEST_CASE( preprocessors, jit_fixture ){
	init( "./repo/question/v1a1/preprocessors.ss" );

	int(*p)() = NULL;
	function( p, "main" );

	BOOST_REQUIRE(p);

	BOOST_CHECK( p() == 0 );
}

BOOST_FIXTURE_TEST_CASE( functions, jit_fixture ){
	init( "./repo/question/v1a1/function.ss" );

	int(*p)(int) = NULL;
	function( p, "foo" );

	BOOST_REQUIRE(p);

	BOOST_CHECK( p(5) == 5 );
}

using eflib::vec3;
using eflib::int2;

BOOST_FIXTURE_TEST_CASE( intrinsics, jit_fixture ){
	init("./repo/question/v1a1/intrinsics.ss");

	float (*test_dot_f3)(vec3, vec3) = NULL;
	function( test_dot_f3, "test_dot_f3" );
	BOOST_REQUIRE(test_dot_f3);
	
	vec3 lhs( 1.0f, 0.0f, 0.0f );
	vec3 rhs( 1.0f, 0.0f, 0.0f );

	float f = test_dot_f3(lhs, rhs);
	BOOST_CHECK_CLOSE( dot_prod3( lhs.xyz(), rhs.xyz() ), f, 0.0001 );

	//int (*test_dot_i2)(int2, int2) = NULL;
	//int2 lhsi( 17, -8 );
	//int2 rhsi( 9, 36 );
	//function( test_dot_i2, "test_dot_i2" );
	//BOOST_REQUIRE(test_dot_i2);

	//BOOST_CHECK_EQUAL( lhsi.x*rhsi.x+lhsi.y*rhsi.y, test_dot_i2(lhsi, rhsi) );
}

//BOOST_FIXTURE_TEST_CASE( booleans, jit_fixture ){
//	init( "./repo/question/v1a1/bool.ss" );
//
//	int(*p)() = static_cast<int(*)()>( je->get_function("Mmain@@") );
//	BOOST_REQUIRE(p);
//
//	BOOST_CHECK( p() == 0 );
//}

BOOST_AUTO_TEST_SUITE_END();