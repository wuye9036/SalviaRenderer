#define ALL_TESTS_ENABLED 0

#include <eflib/include/platform/boost_begin.h>
#include <boost/test/unit_test.hpp>
#include <eflib/include/platform/boost_end.h>

#include <sasl/include/driver/driver_api.h>
#include <sasl/include/common/diag_formatter.h>
#include <sasl/include/code_generator/llvm/cgllvm_api.h>
#include <sasl/include/code_generator/llvm/cgllvm_jit.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/semantic/semantic_infos.h>
#include <salviar/include/shader_abi.h>

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

using sasl::driver::driver;
using sasl::code_generator::jit_engine;
using sasl::code_generator::llvm_module;
using sasl::common::diag_chat;
using sasl::common::diag_item;
using sasl::semantic::symbol;

using salviar::PACKAGE_ELEMENT_COUNT;
using salviar::PACKAGE_LINE_ELEMENT_COUNT;

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
using boost::mpl::transform;

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

bool print_diagnostic( diag_chat*, diag_item* item )
{
	BOOST_MESSAGE( sasl::common::str(item) );
	return true;
}

template <typename Fn>
class jit_function_forward_base{
protected:
	typedef typename result_type<Fn>::type result_t;
	typedef result_t* result_type_pointer;
	typedef typename parameter_types<Fn>::type param_types;
	typedef typename boost::mpl::transform< param_types, if_< or_< is_arithmetic<_>, is_pointer<_> >, _, add_reference<_> > >::type param_refs;
	typedef typename if_<
	is_same<result_t, void>,
	param_refs,
		typename push_front<param_refs, result_type_pointer>::type
	>::type	callee_parameters;
	typedef typename push_front<callee_parameters, void>::type
		callee_return_parameters;
public:
	EFLIB_OPERATOR_BOOL( jit_function_forward_base<Fn> ){ return callee != NULL; }
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

#include <eflib/include/platform/disable_warnings.h>
void invoke( void* callee, void* psi, void* pbi, void* pso, void* pbo )
{
#if defined(EFLIB_CPU_X86) && defined(EFLIB_MSVC)
	__asm{
		push ebp;

		push callee;

		push pbo;
		push pso;
		push pbi;
		push psi;

		mov  ebp, esp ;

		push ebx;
		push esi;
		push edi;

		and  esp, -16;
		sub  esp, 16;

		mov  ebx, [ebp+12];
		push ebx;
		mov  ebx, [ebp+8];
		push ebx;
		mov  ebx, [ebp+4];
		push ebx;
		mov  ebx, [ebp];
		push ebx;

		mov  ebx, [ebp+16];
		call ebx;

		mov  edi, [ebp-12];
		mov  esi, [ebp-8];
		mov  ebx, [ebp-4];
		mov  esp, ebp;
		add  esp, 20;
		pop  ebp;
	}

	// X XXXX
#else
	reinterpret_cast<void (*)(void*, void*, void*, void*)>(callee)( psi, pbi, pso, pbo );
#endif
}
#include <eflib/include/platform/enable_warnings.h>

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


	template <typename T0, typename T1, typename T2, typename T3>
	result_t operator() (T0* psi, T1* pbi, T2* pso, T3* pbo){
		invoke( (void*)callee, psi, pbi, pso, pbo );
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

	void init_ps( string const& file_name ){
		init( file_name, "--lang=ps" );
	}

	void init( string const& file_name, string const& options ){
		diags = diag_chat::create();
		diags->add_report_raised_handler( print_diagnostic );
		sasl_create_driver(drv);
		BOOST_REQUIRE(drv);
		drv->set_diag_chat(diags.get());
		drv->set_parameter( make_command(file_name, options) );
		drv->compile();

		BOOST_REQUIRE( drv->root() );
		BOOST_REQUIRE( drv->mod_si() );
		BOOST_REQUIRE( drv->mod_codegen() );

		root_sym = drv->mod_si()->root();

		shared_ptr<llvm_module> llvm_mod = shared_polymorphic_cast<llvm_module>( drv->mod_codegen() );
		fstream dump_file( ( file_name + "_ir.ll" ).c_str(), std::ios::out );
		llvm_mod->dump( dump_file );
		dump_file.close();

		je = drv->create_jit();
		BOOST_REQUIRE( je );
	}

	template <typename FunctionT>
	void function( FunctionT& fn, string const& unmangled_name ){
		assert( !root_sym->find_overloads(unmangled_name).empty() );
		string fn_name = root_sym->find_overloads(unmangled_name)[0]->mangled_name();
		fn.callee = reinterpret_cast<typename FunctionT::callee_ptr_t>( je->get_function(fn_name) );
	}

	void set_function( void* fn, string const& unmangled_name ){
		assert( !root_sym->find_overloads(unmangled_name).empty() );
		string fn_name = root_sym->find_overloads(unmangled_name)[0]->mangled_name();
		je->inject_function( fn, fn_name );
	}

	~jit_fixture(){}

	shared_ptr<driver>		drv;
	shared_ptr<symbol>		root_sym;
	shared_ptr<jit_engine>	je;
	shared_ptr<diag_chat>	diags;
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

#if ALL_TESTS_ENABLED
BOOST_FIXTURE_TEST_CASE( empty_test, jit_fixture ){
	init_g( "./repo/question/v1a1/empty.ss" );
}
#endif

#if ALL_TESTS_ENABLED
BOOST_FIXTURE_TEST_CASE( comments, jit_fixture ){
	init_g( "./repo/question/v1a1/comments.ss" );
	jit_function<int(int)> fn;
	function( fn, "foo" );

	BOOST_REQUIRE( fn );
	BOOST_CHECK( fn( 1366 ) == 1366 );
}
#endif

#if ALL_TESTS_ENABLED

BOOST_FIXTURE_TEST_CASE( preprocessors, jit_fixture ){
	init_g( "./repo/question/v1a1/preprocessors.ss" );

	jit_function<int()> fn;
	function( fn, "main" );
	BOOST_REQUIRE( fn );

	BOOST_CHECK( fn() == 0 );
}

int fib_ref(int i)
{
	if(i < 2) return i;
	return fib_ref(i-1) + fib_ref(i-2);
}

BOOST_FIXTURE_TEST_CASE( functions, jit_fixture ){
	init_g( "./repo/question/v1a1/function.ss" );

	jit_function<int(int)> foo;
	function( foo, "foo" );
	BOOST_REQUIRE(foo);

	jit_function<int(int, int)> foo2;
	function( foo2, "foo2" );

	jit_function<int(int)> cross_caller;
	function( cross_caller, "cross_caller" );

	jit_function<int(int)> fib;
	function( fib, "fib" );

	BOOST_CHECK_EQUAL( foo(5)			, 5 );
	BOOST_CHECK_EQUAL( foo2(-2, 7)		,-2 );
	BOOST_CHECK_EQUAL( cross_caller(5)	, 5 );
	BOOST_CHECK_EQUAL( fib(16)			, fib_ref(16) );
}

#endif

using eflib::vec3;
using eflib::int2;

#if ALL_TESTS_ENABLED

BOOST_FIXTURE_TEST_CASE( intrinsics, jit_fixture ){
	init_g("./repo/question/v1a1/intrinsics.ss");

	jit_function<float (vec3*, vec3*)> test_dot_f3;
	jit_function<vec4 (mat44*, vec4*)> test_mul_m44v4;
	jit_function<vec4 (mat44*)> test_fetch_m44v4;
	jit_function<float (float) > test_abs_f;
	jit_function<int (int) > test_abs_i;
	jit_function<float (float) > test_exp;
	jit_function<float (float) > test_sqrt_f;
	jit_function<vec2 (vec2) > test_sqrt_f2;
	jit_function<vec3 (vec3, vec3)> test_cross_prod;

	function( test_dot_f3, "test_dot_f3" );
	BOOST_REQUIRE(test_dot_f3);
	
	function( test_mul_m44v4, "test_mul_m44v4" );
	BOOST_REQUIRE( test_mul_m44v4 );

	function( test_abs_f, "test_abs_f" );
	BOOST_REQUIRE( test_abs_f );

	function( test_abs_i, "test_abs_i" );
	BOOST_REQUIRE( test_abs_i );

	function( test_exp, "test_exp" );
	BOOST_REQUIRE( test_exp );

	function( test_sqrt_f, "test_sqrt_f" );
	BOOST_REQUIRE( test_sqrt_f );

	function( test_sqrt_f2, "test_sqrt_f2" );
	BOOST_REQUIRE( test_sqrt_f2 );

	function( test_cross_prod, "test_cross_prod" );
	BOOST_REQUIRE( test_cross_prod );

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
		eflib::transform( refv, mat, rhs );

		BOOST_CHECK_CLOSE( f.x, refv.x, 0.00001f );
		BOOST_CHECK_CLOSE( f.y, refv.y, 0.00001f );
		BOOST_CHECK_CLOSE( f.z, refv.z, 0.00001f );
		BOOST_CHECK_CLOSE( f.w, refv.w, 0.00001f );
	}
	{
		float p = 123.456f;
		BOOST_CHECK_CLOSE( fabsf(p), test_abs_f(p), 0.000001f );

		float n = - p;
		BOOST_CHECK_CLOSE( fabsf(n), test_abs_f(n), 0.000001f );

		float z = 0.0f;
		BOOST_CHECK_CLOSE( fabsf(z), test_abs_f(z), 0.000001f );
	}
	{
		int p = 0x7fffffff;
		BOOST_CHECK_EQUAL( abs(p), test_abs_i(p) );

		int n = - p;
		BOOST_CHECK_EQUAL( abs(n), test_abs_i(n) );

		int z = 0;
		BOOST_CHECK_EQUAL( abs(z), test_abs_i(z) );
	}
	{
		float x;

		x = -10.0f;
		BOOST_CHECK_CLOSE( expf(x), test_exp(x), 0.000001f );

		x = -1.0f;
		BOOST_CHECK_CLOSE( expf(x), test_exp(x), 0.000001f );

		x = 0.0f;
		BOOST_CHECK_CLOSE( expf(x), test_exp(x), 0.000001f );

		x = 1.0f;
		BOOST_CHECK_CLOSE( expf(x), test_exp(x), 0.000001f );

		x = 10.0f;
		BOOST_CHECK_CLOSE( expf(x), test_exp(x), 0.000001f );
	}
	{
		float f = 876.625f;
		BOOST_CHECK_CLOSE( sqrtf(f), test_sqrt_f(f), 0.000001f );

		vec2 v2( 1.7f, 986.27f );
		vec2 sqrt_v2 = test_sqrt_f2( v2 );
		BOOST_CHECK_CLOSE( sqrtf(v2[0]), sqrt_v2[0], 0.000001f );
		BOOST_CHECK_CLOSE( sqrtf(v2[1]), sqrt_v2[1], 0.000001f );
	}
	{
		vec3 v3_a(199.7f, -872.5f, 8.63f);
		vec3 v3_b(-98.7f, -37.29f, 77.3f);

		vec3 cross_v3 = test_cross_prod(v3_a, v3_b);
		vec3 ref_v3 = cross_prod3( v3_a, v3_b );

		BOOST_CHECK_CLOSE( cross_v3[0], ref_v3[0], 0.000001f );
		BOOST_CHECK_CLOSE( cross_v3[1], ref_v3[1], 0.000001f );
		BOOST_CHECK_CLOSE( cross_v3[2], ref_v3[2], 0.000001f );
	}
}

#endif

#if ALL_TESTS_ENABLED

struct intrinsics_vs_data{
	float norm[3];
	float pos[4];
};

struct intrinsics_vs_bout{
	float n_dot_l;
	vec4  pos;
};

struct intrinsics_vs_sin{
	float *position, *normal;
};

struct intrinsics_vs_bin{
	vec3	light;
	float	wvpMat[16];
};

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
	memcpy( &bin.light, &light, sizeof(float)*3 );
	memcpy( &bin.wvpMat[0], &mat, sizeof(float)*16 );

	jit_function<void(intrinsics_vs_sin*, intrinsics_vs_bin*, void*, intrinsics_vs_bout*)> fn;
	function( fn, "fn" );
	BOOST_REQUIRE( fn );
	fn(&sin, &bin, (void*)NULL, &bout);

	vec4 out_pos;
	eflib::transform( out_pos, mat, pos );

	BOOST_CHECK_CLOSE( bout.n_dot_l, dot_prod3( light, norm ), 0.0001f );
	BOOST_CHECK_CLOSE( bout.pos.x, out_pos.x, 0.0001f );
	BOOST_CHECK_CLOSE( bout.pos.y, out_pos.y, 0.0001f );
	BOOST_CHECK_CLOSE( bout.pos.z, out_pos.z, 0.0001f );
	BOOST_CHECK_CLOSE( bout.pos.w, out_pos.w, 0.0001f );
}

#endif

#if ALL_TESTS_ENABLED
BOOST_FIXTURE_TEST_CASE( branches, jit_fixture )
{
	init_g("./repo/question/v1a1/branches.ss");

	jit_function<float (int)> test_if;
	function( test_if, "test_if" );

	BOOST_CHECK_CLOSE( test_if(0), 1.0f, 0.00001f );
	BOOST_CHECK_CLOSE( test_if(12), 0.0f, 0.00001f );
	BOOST_CHECK_CLOSE( test_if(-9), 0.0f, 0.00001f );

	jit_function<int(int, int)> test_for;
	function( test_for, "test_for" );
	BOOST_REQUIRE( test_for );

	BOOST_CHECK_EQUAL( test_for(2,5), 32 );
	BOOST_CHECK_EQUAL( test_for(5,2), 25 );

	jit_function<int(int, int)> test_while;
	function( test_while, "test_while" );
	BOOST_REQUIRE( test_while );

	BOOST_CHECK_EQUAL( test_while(2,5), 32 );
	BOOST_CHECK_EQUAL( test_while(5,2), 25 );

	jit_function<int(int, int)> test_dowhile;
	function( test_dowhile, "test_dowhile" );
	BOOST_REQUIRE( test_dowhile );

	BOOST_CHECK_EQUAL( test_dowhile(2,5), 32 );
	BOOST_CHECK_EQUAL( test_dowhile(5,2), 25 );

	jit_function<int(int, int)> test_switch;
	function( test_switch, "test_switch" );
	BOOST_REQUIRE( test_switch );

	BOOST_CHECK_EQUAL( test_switch(2,0), 1 );
	BOOST_CHECK_EQUAL( test_switch(2,1), 2 );
	BOOST_CHECK_EQUAL( test_switch(2,2), 4 );
	BOOST_CHECK_EQUAL( test_switch(2,3), 8 );
	BOOST_CHECK_EQUAL( test_switch(2,4), 8 );
	BOOST_CHECK_EQUAL( test_switch(2,5), 8876 );
	BOOST_CHECK_EQUAL( test_switch(2,6), 0 );
}
#endif

#if ALL_TESTS_ENABLED

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

#endif

#if ALL_TESTS_ENABLED
BOOST_FIXTURE_TEST_CASE( cast_tests, jit_fixture ){
	init_g( "./repo/question/v1a1/casts.ss" );

	jit_function<int(int)> test_implicit_cast_i32_b;
	function( test_implicit_cast_i32_b, "test_implicit_cast_i32_b" );

	jit_function<float(int)> test_implicit_cast_i32_f32;
	function( test_implicit_cast_i32_f32, "test_implicit_cast_i32_f32" );

	jit_function<int(float)> test_implicit_cast_f32_b;
	function( test_implicit_cast_f32_b, "test_implicit_cast_f32_b" );

	jit_function<float(int, float)> test_op_add_cast;
	function( test_op_add_cast, "test_op_add_cast" );

	jit_function<int(uint8_t, int)> test_op_sub_cast;
	function( test_op_sub_cast, "test_op_sub_cast" );

	jit_function<float(int)> test_sqrt_cast;
	function( test_sqrt_cast, "test_sqrt_cast" );

	BOOST_CHECK_EQUAL( test_implicit_cast_i32_b(0), 85 );
	BOOST_CHECK_EQUAL( test_implicit_cast_i32_b(19), 33 );
	BOOST_CHECK_EQUAL( test_implicit_cast_i32_b(-7), 33 );

	BOOST_CHECK_CLOSE( test_implicit_cast_i32_f32(0), 0.0f, 0.000001f );
	BOOST_CHECK_CLOSE( test_implicit_cast_i32_f32(-20), -20.0f, 0.000001f );
	BOOST_CHECK_CLOSE( test_implicit_cast_i32_f32(17), 17.0f, 0.000001f );

	BOOST_CHECK_EQUAL( test_implicit_cast_f32_b(0.0f), 85 );
	BOOST_CHECK_EQUAL( test_implicit_cast_f32_b(19.0f), 33 );
	BOOST_CHECK_EQUAL( test_implicit_cast_f32_b(-7.0f), 33 );

	BOOST_CHECK_CLOSE( test_op_add_cast( 33,  87.6f),  33+87.6f, 0.000001f );
	BOOST_CHECK_CLOSE( test_op_add_cast(-33,  87.6f), -33+87.6f, 0.000001f );
	BOOST_CHECK_CLOSE( test_op_add_cast( 33, -87.6f),  33-87.6f, 0.000001f );

	BOOST_CHECK_EQUAL( test_op_sub_cast( 122,  8645),  122-8645 );
	BOOST_CHECK_EQUAL( test_op_sub_cast(-122,  8645), -122-8645 );
	BOOST_CHECK_EQUAL( test_op_sub_cast( 122, -8645),  122+8645 );

	BOOST_CHECK_CLOSE( test_sqrt_cast(0),	  0.0f,			   0.000001f );
	BOOST_CHECK_CLOSE( test_sqrt_cast(17652), sqrtf(17652.0f), 0.000001f );
}
#endif

#if ALL_TESTS_ENABLED
BOOST_FIXTURE_TEST_CASE( scalar_tests, jit_fixture ){
	init_ps( "./repo/question/v1a1/scalar.sps" );

	jit_function<void(void*, void*, void*, void*)> ps_main;
	function( ps_main, "ps_main" );
}
#endif

#if ALL_TESTS_ENABLED
BOOST_FIXTURE_TEST_CASE( ps_arith_tests, jit_fixture ){
	init_ps( "./repo/question/v1a1/arithmetic.sps" );

	jit_function<void(void*, void*, void*, void*)> fn;
	function( fn, "fn" );

	float* src	= (float*)_aligned_malloc( PACKAGE_ELEMENT_COUNT * sizeof(float), 16 );
	float* dest	= (float*)_aligned_malloc( PACKAGE_ELEMENT_COUNT * sizeof(float), 16 );
	float dest_ref[PACKAGE_ELEMENT_COUNT];

	for( size_t i = 0; i < PACKAGE_ELEMENT_COUNT; ++i ){
		src[i] = (i+3.77f)*(0.76f*i);

		dest_ref[i] = src[i] + 5.0f;
		dest_ref[i] += (src[i] - 5.0f);
		dest_ref[i] += (src[i] * 5.0f);
	}

	fn( src, (void*)NULL, dest, (void*)NULL );
	
	for( size_t i = 0; i < PACKAGE_ELEMENT_COUNT; ++i ){
		BOOST_CHECK_CLOSE( dest_ref[i], dest[i], 0.000012f );
	}

	_aligned_free( src );
	_aligned_free( dest );
}
#endif

#if ALL_TESTS_ENABLED
BOOST_FIXTURE_TEST_CASE( ps_swz_and_wm, jit_fixture )
{
	init_ps( "./repo/question/v1a1/swizzle_and_wm.sps" );

	jit_function<void(void*, void*, void*, void*)> fn;
	function( fn, "fn" );

	vec4* src	= (vec4*)_aligned_malloc( PACKAGE_ELEMENT_COUNT * sizeof(vec4), 16 );
	vec4* dest	= (vec4*)_aligned_malloc( PACKAGE_ELEMENT_COUNT * sizeof(vec4), 16 );
	vec4 dest_ref[PACKAGE_ELEMENT_COUNT];

	for( size_t i = 0; i < PACKAGE_ELEMENT_COUNT * 4; ++i ){
		((float*)src)[i] = (i+3.77f)*(0.76f*i);
	}

	for( size_t i = 0; i < PACKAGE_ELEMENT_COUNT; ++i ){
		dest_ref[i].yzx( src[i].xyz() + src[i].wxy() );
	}

	fn( src, (void*)NULL, dest, (void*)NULL );

	for( size_t i = 0; i < PACKAGE_ELEMENT_COUNT; ++i ){
		BOOST_CHECK_CLOSE( dest_ref[i].x, dest[i].x, 0.00001f );
		BOOST_CHECK_CLOSE( dest_ref[i].y, dest[i].y, 0.00001f );
		BOOST_CHECK_CLOSE( dest_ref[i].z, dest[i].z, 0.00001f );
	}

	_aligned_free( src );
	_aligned_free( dest );
}
#endif

#if ALL_TESTS_ENABLED

__m128 to_mm( vec4 const& v ){
	__m128 tmp;
	*(vec4*)(&tmp) = v;
	return tmp;
}

vec4 to_vec4( __m128 const& v ){
	return *reinterpret_cast<vec4 const*>(&v);
}

BOOST_FIXTURE_TEST_CASE( ps_intrinsics, jit_fixture )
{
	init_ps( "./repo/question/v1a1/intrinsics.sps" );

	jit_function<void(void*, void*, void*, void*)> fn;
	function( fn, "fn" );

	vec4* in0	= (vec4*)_aligned_malloc( PACKAGE_ELEMENT_COUNT * sizeof(vec4) * 2, 16 );
	vec4* in1	= (vec4*)(in0 + PACKAGE_ELEMENT_COUNT);
	vec4* out0	= (vec4*)_aligned_malloc( PACKAGE_ELEMENT_COUNT * ( sizeof(vec4) + sizeof(vec2) ), 16 );
	vec2* out1	= (vec2*)(out0 + PACKAGE_ELEMENT_COUNT);

	for( size_t i = 0; i < PACKAGE_ELEMENT_COUNT * 4; ++i ){
		((float*)in0)[i] = (i+3.77f)*(0.76f*i);
		((float*)in1)[i] = (i-4.62f)*(0.11f*i);
	}
	
	struct { vec3 out0; vec2 out1; } dest_ref[PACKAGE_ELEMENT_COUNT];
	for( int i = 0; i < PACKAGE_ELEMENT_COUNT; ++i )
	{
		// SSE dot
		vec4 prod = to_vec4( _mm_mul_ps( to_mm(in0[i]), to_mm(in1[i]) ) );
		float x = prod.x + prod.y + prod.z;

		// SSE prod
		__m128 a0 = to_mm(in0[i].yzxw());
		__m128 a1 = to_mm(in1[i].zxyw());
		__m128 b0 = to_mm(in0[i].zxyw());
		__m128 b1 = to_mm(in1[i].yzxw());
		__m128 first_prod = _mm_mul_ps(a0, a1);
		__m128 second_prod = _mm_mul_ps(b0, b1);
		__m128 f3_m = _mm_sub_ps( first_prod, second_prod );
		vec3 f3 = to_vec4( f3_m ).xyz();
		
		dest_ref[i].out0 = to_vec4( _mm_sqrt_ps(to_mm(in0[i])) ).xyz();
		dest_ref[i].out1.x = x;
		dest_ref[i].out1.y = sqrt(in0[i].x);
	}

	fn( (void*)in0, (void*)NULL, (void*)out0, (void*)NULL );

	for( size_t i = 0; i < PACKAGE_ELEMENT_COUNT; ++i ){
		BOOST_CHECK_CLOSE( out0[i].x, dest_ref[i].out0.x, 0.00001f );
		BOOST_CHECK_CLOSE( out0[i].y, dest_ref[i].out0.y, 0.00001f );
		BOOST_CHECK_CLOSE( out0[i].z, dest_ref[i].out0.z, 0.00001f );

		BOOST_CHECK_CLOSE( out1[i].x, dest_ref[i].out1.x, 0.00001f );
		BOOST_CHECK_CLOSE( out1[i].y, dest_ref[i].out1.y, 0.00001f );
	}

	_aligned_free( in0 );
	_aligned_free( out0 );
}
#endif

#if ALL_TESTS_ENABLED 
BOOST_FIXTURE_TEST_CASE( ps_branches, jit_fixture ){
	init_ps( "./repo/question/v1a1/branches.sps" );
	
	jit_function<void(void*, void*, void*, void*)> fn;
	function( fn, "fn" );

	BOOST_REQUIRE( fn );

	float* in0	= (float*)_aligned_malloc( PACKAGE_ELEMENT_COUNT * (sizeof(float) + sizeof(vec4)), 16 );
	vec4* in1	= (vec4*)(in0 + PACKAGE_ELEMENT_COUNT);
	vec2* out	= (vec2*)_aligned_malloc( PACKAGE_ELEMENT_COUNT * sizeof(vec2), 16 );
	vec2 ref_out[ PACKAGE_ELEMENT_COUNT ];

	srand(0);
	for( int i = 0; i < PACKAGE_ELEMENT_COUNT; ++i){
		// Init Data
		in0[i] = (i * 0.34f) - 1.0f;
		for( int j = 0; j < 4; ++j ){
			((float*)(in1))[i*4+j] = rand() / 35.0f;
		}

		// Compute reference data
		ref_out[i].x = 88.3f;
		ref_out[i].y = 75.4f;
		if( in0[i] > 0.0f ){
			ref_out[i].x = in0[i];
		}
		if( in0[i] > 1.0f ){
			ref_out[i].y = in1[i].x;
		} else {
			ref_out[i].y = in1[i].y;
		}
		if( in0[i] > 2.0f ){
			ref_out[i].y = in1[i].z;
			if ( in0[i] > 3.0f ){
				ref_out[i].y = ref_out[i].y + 1.0f;
			} else if( in0[i] > 2.5f ){
				ref_out[i].y = ref_out[i].y + 2.0f;
			}
		}
	}

	fn( (void*)in0, (void*)NULL, (void*)out, (void*)NULL );

	for( size_t i = 0; i < PACKAGE_ELEMENT_COUNT; ++i ){
		BOOST_CHECK_CLOSE( out[i].x, ref_out[i].x, 0.00001f );
		BOOST_CHECK_CLOSE( out[i].y, ref_out[i].y, 0.00001f );
	}

	_aligned_free( in0 );
	_aligned_free( out );
}
#endif

#if ALL_TESTS_ENABLED

template<typename T>
void get_ddx( T* out, T const* in )
{
	int const LINES = PACKAGE_ELEMENT_COUNT / PACKAGE_LINE_ELEMENT_COUNT;

	for( int col = 0; col < PACKAGE_LINE_ELEMENT_COUNT; col+=2 ){
		for( int row = 0; row < LINES; ++row ){
			int index = row*PACKAGE_LINE_ELEMENT_COUNT+col; 
			out[index+1] = out[index] = in[index+1] - in[index];
		}
	}
}

template<typename T>
void get_ddy( T* out, T const* in )
{
	int const LINES = PACKAGE_ELEMENT_COUNT / PACKAGE_LINE_ELEMENT_COUNT;

	for( int row = 0; row < LINES; row+=2 ){
		for( int col = 0; col < PACKAGE_LINE_ELEMENT_COUNT; ++col ){
			int index = row*PACKAGE_LINE_ELEMENT_COUNT+col; 
			out[index+PACKAGE_LINE_ELEMENT_COUNT] = out[index] = in[index+PACKAGE_LINE_ELEMENT_COUNT] - in[index];
		}
	}
}

BOOST_FIXTURE_TEST_CASE( ddx_ddy, jit_fixture ){
	init_ps( "./repo/question/v1a1/ddx_ddy.sps" );

	jit_function<void(void*, void*, void*, void*)> fn;
	function( fn, "fn" );

	BOOST_REQUIRE( fn );

	float* in0 = (float*)_aligned_malloc( PACKAGE_ELEMENT_COUNT * (sizeof(float) + sizeof(vec2) + 2 * sizeof(vec4)), 16 );
	vec2*  in1 = (vec2*)(in0 + PACKAGE_ELEMENT_COUNT);
	vec4*  in2 = (vec4*)(in1 + PACKAGE_ELEMENT_COUNT);
	vec4*  in3 = (vec4*)(in2 + PACKAGE_ELEMENT_COUNT);
	
	float* out0 = (float*)_aligned_malloc( PACKAGE_ELEMENT_COUNT * (sizeof(float) + sizeof(vec2) + 2 * sizeof(vec4)), 16 );
	vec2*  out1 = (vec2*)(out0 + PACKAGE_ELEMENT_COUNT);
	vec4*  out2 = (vec4*)(out1 + PACKAGE_ELEMENT_COUNT);
	vec4*  out3 = (vec4*)(out2 + PACKAGE_ELEMENT_COUNT);

	float ddx_out0[ PACKAGE_ELEMENT_COUNT ];
	vec2  ddx_out1[ PACKAGE_ELEMENT_COUNT ];
	vec4  ddx_out2[ PACKAGE_ELEMENT_COUNT ];
	vec4  ddx_out3[ PACKAGE_ELEMENT_COUNT ];

	float ddy_out0[ PACKAGE_ELEMENT_COUNT ];
	vec2  ddy_out1[ PACKAGE_ELEMENT_COUNT ];
	vec4  ddy_out2[ PACKAGE_ELEMENT_COUNT ];
	vec4  ddy_out3[ PACKAGE_ELEMENT_COUNT ];

	float ref_out0[ PACKAGE_ELEMENT_COUNT ];
	vec2  ref_out1[ PACKAGE_ELEMENT_COUNT ];
	vec4  ref_out2[ PACKAGE_ELEMENT_COUNT ];
	vec4  ref_out3[ PACKAGE_ELEMENT_COUNT ];

	srand(0);

	// Init Data
	for( int i = 0; i < PACKAGE_ELEMENT_COUNT * 11; ++i){
		in0[i] = rand() / 67.0f;
	}

	get_ddx( ddx_out0, in0 );
	get_ddx( ddx_out1, in1 );
	get_ddx( ddx_out2, in2 );
	get_ddx( ddx_out3, in3 );

	get_ddy( ddy_out0, in0 );
	get_ddy( ddy_out1, in1 );
	get_ddy( ddy_out2, in2 );
	get_ddy( ddy_out3, in3 );

	for( int i = 0; i < PACKAGE_ELEMENT_COUNT; ++i ){
		ref_out0[i] = ddx_out0[i]			+ ddy_out0[i];
		ref_out1[i] = ddx_out1[i].xy()		+ ddy_out1[i].yx();
		ref_out2[i] = ddx_out2[i].xyzw()	+ ddy_out2[i].yzxw();
		ref_out3[i] = ddx_out3[i].xwzy()	+ ddy_out3[i].yzxw();
	}

	fn( (void*)in0, (void*)NULL, (void*)out0, (void*)NULL );

	for( size_t i = 0; i < PACKAGE_ELEMENT_COUNT; ++i ){
		BOOST_CHECK_CLOSE( out0[i], ref_out0[i], 0.00001f );
		BOOST_CHECK_CLOSE( out1[i].x, ref_out1[i].x, 0.00001f );
		BOOST_CHECK_CLOSE( out1[i].y, ref_out1[i].y, 0.00001f );
		BOOST_CHECK_CLOSE( out2[i].x, ref_out2[i].x, 0.00001f );
		BOOST_CHECK_CLOSE( out2[i].y, ref_out2[i].y, 0.00001f );
		BOOST_CHECK_CLOSE( out2[i].z, ref_out2[i].z, 0.00001f );
		BOOST_CHECK_CLOSE( out3[i].x, ref_out3[i].x, 0.00001f );
		BOOST_CHECK_CLOSE( out3[i].y, ref_out3[i].y, 0.00001f );
		BOOST_CHECK_CLOSE( out3[i].z, ref_out3[i].z, 0.00001f );
		BOOST_CHECK_CLOSE( out3[i].w, ref_out3[i].w, 0.00001f );
	}

	_aligned_free( in0 );
	_aligned_free( out0 );
}

#endif

#if ALL_TESTS_ENABLED

struct sampler_t{
	uintptr_t ss, tex;
};

void tex2Dlod(vec4* ret, uint16_t mask, sampler_t* s, vec4* t)
{
	BOOST_CHECK_EQUAL( s->ss, 0xF3DE89C );
	BOOST_CHECK_EQUAL( s->tex, 0xB785D3A );

	for( int i = 0; i < PACKAGE_ELEMENT_COUNT; ++i ){
		ret[i] = t[i].zyxw() + t[i].wxzy();
	}
}


BOOST_FIXTURE_TEST_CASE( tex_ps, jit_fixture )
{
	init_ps( "./repo/question/v1a1/tex.sps" );

	set_function( &tex2Dlod, "tex2Dlod" );

	jit_function<void(void*, void*, void*, void*)> fn;
	function( fn, "fn" );

	BOOST_REQUIRE( fn );

	vec4* src	= (vec4*)_aligned_malloc( PACKAGE_ELEMENT_COUNT * sizeof(vec4), 16 );
	vec4* dest	= (vec4*)_aligned_malloc( PACKAGE_ELEMENT_COUNT * sizeof(vec4), 16 );
	vec4  dest_ref[PACKAGE_ELEMENT_COUNT];

	srand(0);
	for( size_t i = 0; i < PACKAGE_ELEMENT_COUNT * 4; ++i ){
		((float*)src)[i] = rand() / 177.8f;
	}

	for( size_t i = 0; i < PACKAGE_ELEMENT_COUNT; ++i ){
		dest_ref[i] = src[i].zyxw() + src[i].wxzy();
	}
	sampler_t smpr;
	
	smpr.ss = 0xF3DE89C;
	smpr.tex = 0xB785D3A;

	intptr_t addr = reinterpret_cast<intptr_t>(&smpr);
	fn( src, (void*)&addr, dest, (void*)NULL );

	for( size_t i = 0; i < PACKAGE_ELEMENT_COUNT; ++i ){
		BOOST_CHECK_CLOSE( dest_ref[i].x, dest[i].x, 0.00001f );
		BOOST_CHECK_CLOSE( dest_ref[i].y, dest[i].y, 0.00001f );
		BOOST_CHECK_CLOSE( dest_ref[i].z, dest[i].z, 0.00001f );
	}

	_aligned_free( src );
	_aligned_free( dest );
}

#endif

#if ALL_TESTS_ENABLED

BOOST_FIXTURE_TEST_CASE( ps_for_loop, jit_fixture ){
	init_ps( "./repo/question/v1a1/for_loop.sps" );

	jit_function<void(void*, void*, void*, void*)> fn;
	function( fn, "fn" );

	BOOST_REQUIRE( fn );

	float* in	= (float*)_aligned_malloc( PACKAGE_ELEMENT_COUNT * sizeof(float), 16 );
	float* out	= (float*)_aligned_malloc( PACKAGE_ELEMENT_COUNT * sizeof(float), 16 );
	float ref_out[ PACKAGE_ELEMENT_COUNT ];

	srand(0);
	for( int i = 0; i < PACKAGE_ELEMENT_COUNT; ++i){
		// Init Data
		in[i] = rand() / 1000.0f;

		float x = in[i];
		for( int j = 0; j < 10; ++j )
		{
			x *= 2.0f;
			if ( x > 5000.0f ){ break; }
		}

		ref_out[i] = x;
	}

	fn( (void*)in, (void*)NULL, (void*)out, (void*)NULL );

	for( size_t i = 0; i < PACKAGE_ELEMENT_COUNT; ++i ){
		BOOST_CHECK_CLOSE( out[i], ref_out[i], 0.00001f );
		BOOST_CHECK_CLOSE( out[i], ref_out[i], 0.00001f );
	}

	_aligned_free( in );
	_aligned_free( out );
}

#endif

#if ALL_TESTS_ENABLED

BOOST_FIXTURE_TEST_CASE( ps_while, jit_fixture ){
	init_ps( "./repo/question/v1a1/while.sps" );

	jit_function<void(void*, void*, void*, void*)> fn;
	function( fn, "fn" );

	BOOST_REQUIRE( fn );

	float* in	= (float*)_aligned_malloc( PACKAGE_ELEMENT_COUNT * sizeof(float), 16 );
	float* out	= (float*)_aligned_malloc( PACKAGE_ELEMENT_COUNT * sizeof(float), 16 );
	float ref_out[ PACKAGE_ELEMENT_COUNT ];

	srand(0);
	for( int i = 0; i < PACKAGE_ELEMENT_COUNT; ++i){
		// Init Data
		in[i] = 0.79f * ( i * i * i );

		float x = in[i];
		while( x < 3000.0f )
		{
			if( x < 1.0f ) {
				break;
			}
			x *= x;
		}

		ref_out[i] = x;
	}

	fn( (void*)in, (void*)NULL, (void*)out, (void*)NULL );

	for( size_t i = 0; i < PACKAGE_ELEMENT_COUNT; ++i ){
		BOOST_CHECK_CLOSE( out[i], ref_out[i], 0.00001f );
		BOOST_CHECK_CLOSE( out[i], ref_out[i], 0.00001f );
	}

	_aligned_free( in );
	_aligned_free( out );
}

#endif

#if ALL_TESTS_ENABLED

BOOST_FIXTURE_TEST_CASE( ps_do_while, jit_fixture ){
	init_ps( "./repo/question/v1a1/do_while.sps" );

	jit_function<void(void*, void*, void*, void*)> fn;
	function( fn, "fn" );

	BOOST_REQUIRE( fn );

	float* in	= (float*)_aligned_malloc( PACKAGE_ELEMENT_COUNT * sizeof(float), 16 );
	float* out	= (float*)_aligned_malloc( PACKAGE_ELEMENT_COUNT * sizeof(float), 16 );
	float ref_out[ PACKAGE_ELEMENT_COUNT ];

	srand(0);
	for( int i = 0; i < PACKAGE_ELEMENT_COUNT; ++i){
		// Init Data
		in[i] = 0.79f * ( i * i * i );

		float x = in[i];
		do {
			if( x < 1.0f ) {
				break;
			}
			x *= x;
		} while( x < 3000.0f );

		ref_out[i] = x;
	}

	fn( (void*)in, (void*)NULL, (void*)out, (void*)NULL );

	for( size_t i = 0; i < PACKAGE_ELEMENT_COUNT; ++i ){
		BOOST_CHECK_CLOSE( out[i], ref_out[i], 0.00001f );
		BOOST_CHECK_CLOSE( out[i], ref_out[i], 0.00001f );
	}

	_aligned_free( in );
	_aligned_free( out );
}

#endif

#if ALL_TESTS_ENABLED

BOOST_FIXTURE_TEST_CASE( constructor_ss, jit_fixture ){
	init_g( "./repo/question/v1a1/constructors.ss" );

	jit_function<int2()> get_int2;
	function( get_int2, "get_int2" );

	jit_function<int3( int3 )> add_int3;
	function( add_int3, "add_int3" );

	jit_function<vec3()> get_float3;
	function( get_float3, "get_float3" );

	jit_function<vec4( vec4 )> add_float4;
	function( add_float4, "add_float4" );

	BOOST_REQUIRE( get_int2 );
	BOOST_REQUIRE( add_int3 );
	BOOST_REQUIRE( get_float3 );
	BOOST_REQUIRE( add_float4 );

	int2 ref_v0 = int2(95, 86);
	int3 ref_v1 = int3(22, 95, 86) + int3(67, 92, -98);
	vec3 ref_v2 = vec3(0.87f, 7.89f, 98.76f);
	vec4 ref_v3 = vec4(1.22f, 0.93f, 187.22f, 5.56f) + vec4(3.3f, 6.7f, 90.2f, -8.8f);

	int2 v0 = get_int2();
	int3 v1 = add_int3( int3(67, 92, -98) );
	vec3 v2 = get_float3();
	vec4 v3 = add_float4( vec4(3.3f, 6.7f, 90.2f, -8.8f) );

	BOOST_CHECK_EQUAL( ref_v0.x, v0.x );
	BOOST_CHECK_EQUAL( ref_v0.y, v0.y );

	BOOST_CHECK_EQUAL( ref_v1.x, v1.x );
	BOOST_CHECK_EQUAL( ref_v1.y, v1.y );
	BOOST_CHECK_EQUAL( ref_v1.z, v1.z );

	BOOST_CHECK_CLOSE( ref_v2.x, v2.x, 0.000001f );
	BOOST_CHECK_CLOSE( ref_v2.y, v2.y, 0.000001f );
	BOOST_CHECK_CLOSE( ref_v2.z, v2.z, 0.000001f );

	BOOST_CHECK_CLOSE( ref_v3.x, v3.x, 0.000001f );
	BOOST_CHECK_CLOSE( ref_v3.y, v3.y, 0.000001f );
	BOOST_CHECK_CLOSE( ref_v3.z, v3.z, 0.000001f );
	BOOST_CHECK_CLOSE( ref_v3.w, v3.w, 0.000001f );

	return ;
}

#endif

#if ALL_TESTS_ENABLED
BOOST_FIXTURE_TEST_CASE( local_var, jit_fixture ){
	init_g( "./repo/question/v1a1/local_var.ss" );

	jit_function<int(int, int)> get_sum;
	function( get_sum, "get_sum" );

	BOOST_REQUIRE( get_sum );

	BOOST_CHECK_EQUAL( get_sum(10,       2),       10* 2 );
	BOOST_CHECK_EQUAL( get_sum(987,      3),      987* 3 );
	BOOST_CHECK_EQUAL( get_sum(22876765, 1),  22876765*1 );
}
#endif

#if 1 || ALL_TESTS_ENABLED
BOOST_FIXTURE_TEST_CASE( arith_ops, jit_fixture )
{
	init_g( "./repo/question/v1a1/arithmetic.ss" );

	jit_function<vec4(vec4)> test_float_arith;
	function( test_float_arith, "test_float_arith" );
	BOOST_REQUIRE(test_float_arith);

	jit_function<int3(int3)> test_int_arith;
	function( test_int_arith, "test_int_arith" );
	BOOST_REQUIRE(test_int_arith);

	vec4 vf( 76.8f, -88.5f, 37.7f, -98.1f );
	int3 vi( 87, 46, 22 );

	vec4 ref_f( vf.x+vf.y, vf.y-vf.z, vf.z*vf.w, vf.w/vf.x );
	vec4 ref_i( vi.x/vi.y, vi.y%vi.z, vi.z*vi.x );

	vec4 ret_f = test_float_arith(vf);
	int3 ret_i = test_int_arith(vi);

	BOOST_CHECK_CLOSE( ref_f.x, ret_f.x, 0.000001f );
	BOOST_CHECK_CLOSE( ref_f.y, ret_f.y, 0.000001f );
	BOOST_CHECK_CLOSE( ref_f.z, ret_f.z, 0.000001f );
	BOOST_CHECK_CLOSE( ref_f.w, ret_f.w, 0.000001f );

	BOOST_CHECK_EQUAL( ref_i.x, ret_i.x );
	BOOST_CHECK_EQUAL( ref_i.y, ret_i.y );
	BOOST_CHECK_EQUAL( ref_i.z, ret_i.z );
}
#endif

BOOST_AUTO_TEST_SUITE_END();
