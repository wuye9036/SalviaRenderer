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
#include <eflib/include/metaprog/util.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/function.hpp>
#include <boost/function_types/function_type.hpp>
#include <boost/function_types/function_pointer.hpp>
#include <boost/function_types/result_type.hpp>
#include <boost/function_types/parameter_types.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/push_front.hpp>
#include <boost/mpl/or.hpp>
#include <boost/type_traits/is_arithmetic.hpp>
#include <boost/type_traits/add_reference.hpp>
#include <boost/type_traits/is_pointer.hpp>
#include <boost/type_traits/is_same.hpp>
#include <eflib/include/platform/boost_end.h>

#include <eflib/include/platform/cpuinfo.h>

#include <fstream>
#include <iostream>

using namespace eflib;
using sasl::compiler::compiler;

using sasl::semantic::symbol;

using sasl::code_generator::jit_engine;
using sasl::code_generator::cgllvm_jit_engine;
using sasl::code_generator::llvm_module;

using boost::shared_ptr;
using boost::shared_polymorphic_cast;
using boost::function_types::result_type;
using boost::function_types::function_pointer;
using boost::function_types::parameter_types;

using std::fstream;
using std::string;
using std::cout;
using std::endl;

using boost::mpl::_;
using boost::mpl::if_;
using boost::mpl::or_;
using boost::mpl::push_front;
using boost::mpl::sizeof_;

using boost::is_arithmetic;
using boost::is_pointer;
using boost::is_same;

using boost::add_reference;
using boost::enable_if_c;
using boost::enable_if;
using boost::disable_if;

BOOST_AUTO_TEST_SUITE( jit )

string make_command( string const& file_name, string const& options){
	return "--input=\"" + file_name + "\" " + options;
}

template <typename Fn>
class jit_function_forward_base{
protected:
	typedef typename result_type<Fn>::type result_t;
	typedef result_t* result_type_pointer;
	typedef if_< or_< is_arithmetic<_>, is_pointer<_> >, _, add_reference<_> >
		parameter_type_convertor;
	typedef typename parameter_types<Fn, parameter_type_convertor>::type
		param_types;
	typedef typename if_<
		is_same<result_t, void>,
		param_types,
		typename push_front<param_types, result_type_pointer>::type
	>::type	callee_parameters;
	typedef typename push_front<callee_parameters, void>::type
		callee_return_parameters;
public:
	EFLIB_OPERATOR_BOOL( jit_function_forward_base<Fn> ){ return callee; }
	typedef typename function_pointer<callee_return_parameters>::type
		callee_ptr_t;
	callee_ptr_t callee;
	jit_function_forward_base():callee(NULL){}
};

template <typename RT, typename Fn>
class jit_function_forward: public jit_function_forward_base<Fn>{
public:
	result_t operator ()(){
		result_t tmp;
		callee(&tmp);
		return tmp;
	}

	template <typename T0>
	result_t operator() (T0 p0 ){
		result_t tmp;
		callee(&tmp, p0);
		return tmp;
	}

	template <typename T0, typename T1>
	result_t operator() (T0 p0, T1 p1){
		result_t tmp;
		callee(&tmp, p0, p1);
		return tmp;
	}

	template <typename T0, typename T1, typename T2>
	result_t operator() (T0 p0, T1 p1, T2 p2){
		result_t tmp;
		callee(&tmp, p0, p1, p2);
		return tmp;
	}
};

template <typename Fn>
class jit_function_forward<void, Fn>: public jit_function_forward_base<Fn>{
public:
	result_t operator ()(){
		callee();
	}

	template <typename T0>
	result_t operator() (T0 p0 ){
		callee(p0);
	}

	template <typename T0, typename T1>
	result_t operator() (T0 p0, T1 p1){
		callee(p0, p1);
	}

	template <typename T0, typename T1, typename T2>
	result_t operator() (T0 p0, T1 p1, T2 p2){
		callee(p0, p1, p2);
	}

	template <typename T0, typename T1, typename T2, typename T3>
	result_t operator() (T0 p0, T1 p1, T2 p2, T3 p3){
		callee(p0, p1, p2, p3);
	}
};

template <typename Fn>
class jit_function: public jit_function_forward< typename result_type<Fn>::type, Fn >
{};

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
		assert( !root_sym->find_overloads(unmangled_name).empty() );
		string fn_name = root_sym->find_overloads(unmangled_name)[0]->mangled_name();
		fn.callee = reinterpret_cast<typename FunctionT::callee_ptr_t>( je->get_function(fn_name) );
	}

	~jit_fixture(){}

	compiler c;
	shared_ptr<symbol> root_sym;
	shared_ptr<jit_engine> je;
};


BOOST_AUTO_TEST_CASE( detect_cpu_features ){
	cout << endl << "================================================" << endl << endl;
	cout << "Detecting CPU Features... " << endl;

	if( support_feature(cpu_sse2) ){
		cout << "    ... SSE2 Detected" << endl;
	} 
	if( support_feature(cpu_sse3) ){
		cout << "    ... SSE3 Detected" << endl;
	}
	if( support_feature(cpu_ssse3) ){
		cout << "    ... Supplemental SSE3 Detected" << endl;
	} 
	if( support_feature(cpu_sse41) ){
		cout << "    ... SSE4.1 Detected" << endl;
	} 
	if( support_feature(cpu_sse42) ){
		cout << "    ... SSE4.2 Detected" << endl;
	}
	if( support_feature(cpu_sse4a) ){
		cout << "    ... SSE4A Detected" << endl;
	}
	if( support_feature(cpu_avx) ){
		cout << "    ... AVX Detected" << endl;
	}

	cout << endl << "================================================" << endl << endl;

	BOOST_CHECK(true);
}

#if 0

BOOST_FIXTURE_TEST_CASE( empty_test, jit_fixture ){
	init_g( "./repo/question/v1a1/empty.ss" );
}

BOOST_FIXTURE_TEST_CASE( comments, jit_fixture ){
	init_g( "./repo/question/v1a1/comments.ss" );
	jit_function<int(int)> fn;
	function( fn, "foo" );

	BOOST_REQUIRE( fn );
	BOOST_CHECK( fn( 1366 ) == 1366 );
}

BOOST_FIXTURE_TEST_CASE( preprocessors, jit_fixture ){
	init_g( "./repo/question/v1a1/preprocessors.ss" );

	jit_function<int()> fn;
	function( fn, "main" );
	BOOST_REQUIRE( fn );

	BOOST_CHECK( fn() == 0 );
}

BOOST_FIXTURE_TEST_CASE( functions, jit_fixture ){
	init_g( "./repo/question/v1a1/function.ss" );

	jit_function<int(int)> fn;
	function( fn, "foo" );
	BOOST_REQUIRE(fn);

	BOOST_CHECK	( fn(5) == 5 );
}

#endif

using eflib::vec3;
using eflib::int2;

BOOST_FIXTURE_TEST_CASE( intrinsics, jit_fixture ){
	init_g("./repo/question/v1a1/intrinsics.ss");

	jit_function<float (vec3*, vec3*)> test_dot_f3;
	jit_function<vec4 (mat44*, vec4*)> test_mul_m44v4;
	jit_function<vec4 (mat44*)> test_fetch_m44v4;
	jit_function<float (float) > test_sqrt_f;

	function( test_dot_f3, "test_dot_f3" );
	BOOST_REQUIRE(test_dot_f3);
	
	function( test_mul_m44v4, "test_mul_m44v4" );
	BOOST_REQUIRE( test_mul_m44v4 );

	function( test_sqrt_f, "test_sqrt_f" );
	BOOST_REQUIRE( test_sqrt_f );

	{
		vec3 lhs( 4.0f, 9.3f, -5.9f );
		vec3 rhs( 1.0f, -22.0f, 8.28f );

		float f = test_dot_f3(&lhs, &rhs);
		BOOST_CHECK_CLOSE( dot_prod3( lhs.xyz(), rhs.xyz() ), f, 0.0001 );
	}

	{
		mat44 mat( mat44::identity() );
		mat.f[0][0] = 1.0f;
		mat.f[0][1] = 1.0f;
		mat.f[0][2] = 1.0f;
		mat.f[0][3] = 1.0f;

		for( int i = 0; i < 16; ++i){
			((float*)(&mat))[i] = float(i);
		}
		mat44 tmpMat;
		mat_mul( mat, mat_rotX(tmpMat, 0.2f ), mat );
		mat_mul( mat, mat_rotY(tmpMat, -0.3f ), mat );
		mat_mul( mat, mat_translate(tmpMat, 1.7f, -0.9f, 1.1f ), mat );
		mat_mul( mat, mat_scale(tmpMat, 0.5f, 1.2f, 2.0f), mat );

		vec4 rhs( 1.0f, 2.0f, 3.0f, 4.0f );

		vec4 f = test_mul_m44v4(&mat, &rhs);
		vec4 refv;
		transform( refv, mat, rhs );

		BOOST_CHECK_CLOSE( f.x, refv.x, 0.0001f );
		BOOST_CHECK_CLOSE( f.y, refv.y, 0.001f );
		BOOST_CHECK_CLOSE( f.z, refv.z, 0.001f );
		BOOST_CHECK_CLOSE( f.w, refv.w, 0.001f );
	}
	{
		float f = 876.625f;
		BOOST_CHECK_CLOSE( sqrtf(f), test_sqrt_f(f), 0.0001f );
	}
}

#if 0

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
	

	vec4 pos(3.0f, 4.5f, 2.6f, 1.0f);
	vec3 norm(1.5f, 1.2f, 0.8f);
	vec3 light(0.6f, 1.1f, 4.7f);

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

	jit_function<void(intrinsics_vs_sin*, intrinsics_vs_bin*, void*, intrinsics_vs_bout*)> fn;
	function( fn, "fn" );
	BOOST_REQUIRE( fn );
	fn(&sin, &bin, (void*)NULL, &bout);

	vec4 out_pos;
	transform( out_pos, mat, pos );

	BOOST_CHECK_CLOSE( bout.n_dot_l, dot_prod3( light, norm ), 0.0001f );
	BOOST_CHECK_CLOSE( bout.x, out_pos.x, 0.0001f );
	BOOST_CHECK_CLOSE( bout.y, out_pos.y, 0.0001f );
	BOOST_CHECK_CLOSE( bout.z, out_pos.z, 0.0001f );
	BOOST_CHECK_CLOSE( bout.w, out_pos.w, 0.0001f );
}

BOOST_FIXTURE_TEST_CASE( branches, jit_fixture )
{
	init_g("./repo/question/v1a1/branches.ss");

	jit_function<float (int)> test_if;
	function( test_if, "test_if" );

	BOOST_CHECK_CLOSE( test_if(0), 1.0f, 0.00001f );
	BOOST_CHECK_CLOSE( test_if(12), 0.0f, 0.00001f );
	BOOST_CHECK_CLOSE( test_if(-9), 0.0f, 0.00001f );
}

bool test_short_ref(int i, int j, int k){
	return ( i == 0 || j == 0) && k!= 0;
}

BOOST_FIXTURE_TEST_CASE( bool_test, jit_fixture )
{
	init_g( "./repo/question/v1a1/bool.ss" );

	jit_function<int(int, int)> test_max, test_min;
	jit_function<bool(int, int)> test_le;
	jit_function<bool(float, float)> test_ge;
	jit_function<bool(int, int, int)> test_short;

	function( test_max, "test_max" );
	function( test_min, "test_min" );
	function( test_le, "test_le" );
	function( test_ge, "test_ge" );
	function( test_short, "test_short" );

	BOOST_CHECK( test_max( 2, 15 ) == 15 );
	BOOST_CHECK( test_max( 15, 2 ) == 15 );
	BOOST_CHECK( test_min( 2, 15 ) == 2 );
	BOOST_CHECK( test_min( 15, 2 ) == 2 );

	BOOST_CHECK( test_le( 5, 6 ) );
	BOOST_CHECK( test_le( 6, 6 ) );
	BOOST_CHECK( !test_le( 6, 5 ) );

	BOOST_CHECK( !test_ge( 5, 6 ) );
	BOOST_CHECK( test_ge( 6, 6 ) );
	BOOST_CHECK( test_ge( 6, 5 ) );

	for( int i = -1; i < 2; ++i ){
		for( int j = -1; j < 2; ++j ){
			for( int k = -1; k < 2; ++k ){
				bool short_result = test_short(i, j, k);
				bool ref_result = test_short_ref(i, j, k);
				BOOST_CHECK_EQUAL( short_result, ref_result );
			}
		}
	}
}

BOOST_FIXTURE_TEST_CASE( unary_operators_test, jit_fixture )
{
	init_g( "./repo/question/v1a1/unary_operators.ss" );

	jit_function<int(int)> test_pre_inc, test_pre_dec, test_post_inc, test_post_dec;
	function( test_pre_inc, "test_pre_inc" );
	BOOST_REQUIRE(test_pre_inc);
	function( test_pre_dec, "test_pre_dec" );
	BOOST_REQUIRE(test_pre_dec);
	function( test_post_inc, "test_post_inc" );
	BOOST_REQUIRE(test_post_inc);
	function( test_post_dec, "test_post_dec" );
	BOOST_REQUIRE(test_post_dec);

	BOOST_CHECK( test_pre_inc(5) == 13 );
	BOOST_CHECK( test_pre_dec(5) == 7 );
	BOOST_CHECK( test_post_inc(5) == 11 );
	BOOST_CHECK( test_post_dec(5) == 9 );
}

#endif

BOOST_FIXTURE_TEST_CASE( initializer_test, jit_fixture ){
	init_g( "./repo/question/v1a1/initializer.ss" );

	jit_function<int()> test_exprinit;
	jit_function<float(float,float)> test_exprinit2;

	function( test_exprinit, "test_exprinit" );
	BOOST_REQUIRE( test_exprinit );
	function( test_exprinit2, "test_exprinit2" );
	BOOST_REQUIRE( test_exprinit2 );

	BOOST_CHECK_EQUAL( test_exprinit(), 8 );
	BOOST_CHECK_EQUAL( ( test_exprinit2(9.8f, 7.6f) ), 9.8f+7.6f );
}

BOOST_AUTO_TEST_SUITE_END();