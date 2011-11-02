#include <eflib/include/platform/boost_begin.h>
#include <boost/test/unit_test.hpp>
#include <eflib/include/platform/boost_end.h>

#include <sasl/include/compiler/options.h>
#include <sasl/include/code_generator/llvm/cgllvm_api.h>
#include <sasl/include/code_generator/llvm/cgllvm_jit.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/semantic/semantic_infos.h>

#include <eflib/include/math/vector.h>
#include <eflib/include/math/matrix.h>

#include <fstream>

using namespace eflib;
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

string make_command( string const& file_name, string const& options){
	return "--input=\"" + file_name + "\" " + options;
}

struct jit_fixture {
	
	jit_fixture() {}

	void init_g( string const& file_name ){
		init( file_name, "--lang=g" );
	}

	void init_vs( string const& file_name ){
		init( file_name, "--lang=vs" );
	}

	void init( string const& file_name, string const& options ){
		c.parse( make_command(file_name, options) );

		bool aborted = false;
		c.process( aborted );

		BOOST_REQUIRE( c.root() );
		BOOST_REQUIRE( c.module_sem() );
		BOOST_REQUIRE( c.module_codegen() );

		root_sym = c.module_sem()->root();

		fstream dump_file( ( file_name + "_ir.ll" ).c_str(), std::ios::out );
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
	init_g( "./repo/question/v1a1/preprocessors.ss" );

	int(*__stdcall p)() = NULL;
	function( p, "main" );

	BOOST_REQUIRE(p);

	BOOST_CHECK( p() == 0 );
}

BOOST_FIXTURE_TEST_CASE( functions, jit_fixture ){
	init_g( "./repo/question/v1a1/function.ss" );

	int(*__fastcall p)(int) = NULL;
	function( p, "foo" );

	BOOST_REQUIRE(p);

	BOOST_CHECK( p(5) == 5 );
}

using eflib::vec3;
using eflib::int2;

BOOST_FIXTURE_TEST_CASE( intrinsics, jit_fixture ){
	init_g("./repo/question/v1a1/intrinsics.ss");

	float (*__fastcall test_dot_f3)(vec3*, vec3*) = NULL;
	vec4 (*__fastcall test_mul_m44v4)(void*, void*) = NULL;
	vec4 (*__fastcall test_fetch_m44v4)(void*, void*) = NULL;

	function( test_dot_f3, "test_dot_f3" );
	BOOST_REQUIRE(test_dot_f3);
	
	function( test_mul_m44v4, "test_mul_m44v4" );
	BOOST_REQUIRE( test_mul_m44v4 );

	function( test_fetch_m44v4, "test_fetch_m44v4" );
	BOOST_REQUIRE( test_fetch_m44v4 );
	{
		vec3 lhs( 4.0f, 9.3f, -5.9f );
		vec3 rhs( 1.0f, -22.0f, 8.28f );

		float f = test_dot_f3(&lhs, &rhs);
		BOOST_CHECK_CLOSE( dot_prod3( lhs.xyz(), rhs.xyz() ), f, 0.0001 );
	}
	
	{
		mat44 lhs( mat44::identity() );
		vec4 rhs( 1.0f, 2.0f, 3.0f, 4.0f );

		vec4 f = test_fetch_m44v4(&lhs, &rhs);
		vec4 refv = lhs.get_row(0);

		BOOST_CHECK_CLOSE( f.x, refv.x, 0.0001f );
		BOOST_CHECK_CLOSE( f.y, refv.y, 0.001f );
		BOOST_CHECK_CLOSE( f.z, refv.z, 0.001f );
		BOOST_CHECK_CLOSE( f.w, refv.w, 0.001f );
	}

	{
		mat44 lhs( mat44::identity() );
		vec4 rhs( 1.0f, 2.0f, 3.0f, 4.0f );

		vec4 f = test_mul_m44v4(&lhs, &rhs);
		vec4 refv;
		transform( refv, lhs, rhs );

		BOOST_CHECK_CLOSE( f.x, refv.x, 0.0001f );
		BOOST_CHECK_CLOSE( f.y, refv.y, 0.001f );
		BOOST_CHECK_CLOSE( f.z, refv.z, 0.001f );
		BOOST_CHECK_CLOSE( f.w, refv.w, 0.001f );
	}
	//int (*test_dot_i2)(int2, int2) = NULL;
	//int2 lhsi( 17, -8 );
	//int2 rhsi( 9, 36 );
	//function( test_dot_i2, "test_dot_i2" );
	//BOOST_REQUIRE(test_dot_i2);

	//BOOST_CHECK_EQUAL( lhsi.x*rhsi.x+lhsi.y*rhsi.y, test_dot_i2(lhsi, rhsi) );
}

#pragma pack(push)
#pragma pack(1)

struct intrinsics_vs_data{
	float pos[4];
	float norm[3];
};

struct intrinsics_vs_bout{
	float x,y,z,w;
	float n_dot_l;
};

struct intrinsics_vs_sin{
	float *position, *normal;
};

struct intrinsics_vs_bin{
	float wvpMat[16];
	float lx, ly, lz;
};

#pragma pack(pop)

BOOST_FIXTURE_TEST_CASE( intrinsics_vs, jit_fixture ){
	init_vs("./repo/question/v1a1/intrinsics.svs");
	
	intrinsics_vs_data data;
	intrinsics_vs_sin sin;
	sin.position = &( data.pos[0] );
	sin.normal = &(data.norm[0]);
	intrinsics_vs_bin bin;
	intrinsics_vs_bout bout;
	
	void (*fn)( intrinsics_vs_sin*, intrinsics_vs_bin*, void*, intrinsics_vs_bout*) = NULL;
	vec4 pos(3.0f, 4.5f, 2.6f, 1.0f);
	vec3 norm(1.5f, 1.2f, 0.8f);
	vec3 light(0.6f, 1.1f, 4.7f);

	//vec4 pos(0.0f, 0.0f, 0.0f, 0.0f);
	//vec3 norm(0.0f, 0.0f, 0.0f);
	//vec3 light(0.0f, 0.0f, 0.0f);


	mat44 mat( mat44::identity() );
	mat44 tmpMat;
	mat_mul( mat, mat_rotX(tmpMat, 0.2f ), mat );
	mat_mul( mat, mat_rotY(tmpMat, -0.3f ), mat );
	mat_mul( mat, mat_translate(tmpMat, 1.7f, -0.9f, 1.1f ), mat );
	mat_mul( mat, mat_scale(tmpMat, 0.5f, 1.2f, 2.0f), mat );

	memcpy( sin.position, &pos, sizeof(float)*4 );
	memcpy( sin.normal, &norm, sizeof(float)*3 );
	memcpy( &bin.lx, &light, sizeof(float)*3 );
	memcpy( &bin.wvpMat[0], &mat, sizeof(float)*16 );

	function( fn, "fn" );
	BOOST_REQUIRE( fn );

	fn(&sin, &bin, NULL, &bout);
	vec4 out_pos;
	transform( out_pos, mat, pos );

	BOOST_CHECK_CLOSE( bout.n_dot_l, dot_prod3( light, norm ), 0.0001f );
	BOOST_CHECK_CLOSE( bout.x, out_pos.x, 0.0001f );
	BOOST_CHECK_CLOSE( bout.y, out_pos.y, 0.0001f );
	BOOST_CHECK_CLOSE( bout.z, out_pos.z, 0.0001f );
	BOOST_CHECK_CLOSE( bout.w, out_pos.w, 0.0001f );
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