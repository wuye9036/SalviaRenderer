#define ALL_TESTS_ENABLED 0

#include <eflib/include/platform/boost_begin.h>
#include <boost/test/unit_test.hpp>
#include <eflib/include/platform/boost_end.h>

#include <sasl/test/jit_test/jit_test.h>
#include <salviar/include/shader_abi.h>
#include <eflib/include/platform/cpuinfo.h>
#include <eflib/include/diagnostics/profiler.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/math/special_functions/fpclassify.hpp>
#include <eflib/include/platform/boost_end.h>

#include <iostream>

using salviar::PACKAGE_ELEMENT_COUNT;
using salviar::PACKAGE_LINE_ELEMENT_COUNT;
using namespace eflib;
using std::cout;
using std::endl;
using std::numeric_limits;

int const SIMD_ALIGNMENT = 32;

uint32_t naive_reversebits(uint32_t v)
{
	uint32_t ret = 0;
	for(int i = 0; i < 32; ++i)
	{
		ret |= ( ( ( v & (1<<i) ) >> i ) << (31-i) );
	}
	return ret;
}

vec4 lit(float n_dot_l, float n_dot_h, float m)
{
	return vec4(
		1.0f,
		std::max(n_dot_l, 0.0f), 
		(n_dot_l < 0.0f) || (n_dot_h < 0.0f) ? 0.0f : (n_dot_h*m),
		1.0f
		);
}

vec3 faceforward(vec3 n, vec3 i, vec3 ng)
{
	return -n*eflib::sign(eflib::dot_prod3(i, ng));
}
BOOST_AUTO_TEST_SUITE( jit )

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

#if 1
BOOST_AUTO_TEST_CASE( test_profiler )
{
	profiler prof;

	{
		profiling_scope ps(&prof, "A");
		{
			{
				profiling_scope ps(&prof, "B");
			}
			{
				profiling_scope ps(&prof, "SooooooooooooooooooooooooooooooooooooooooLoooooooooooooooooooooooongName");
			}
			{
				profiling_scope ps(&prof, "C");
			}
		}
	}

	print_profiler(&prof, 3);
}
#endif

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
#endif

#if ALL_TESTS_ENABLED

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

#if 1 || ALL_TESTS_ENABLED

BOOST_FIXTURE_TEST_CASE( intrinsics, jit_fixture ){
	init_g("./repo/question/v1a1/intrinsics.ss");
#if ALL_TESTS_ENABLED
	JIT_FUNCTION(float (vec3*, vec3*), test_dot_f3);
	JIT_FUNCTION(vec4 (mat44*, vec4*), test_mul_m44v4);
	JIT_FUNCTION(vec4 (mat44*), test_fetch_m44v4);
	JIT_FUNCTION(float (float), test_abs_f);
	JIT_FUNCTION(int (int), test_abs_i);
	JIT_FUNCTION(float (float), test_exp);
	JIT_FUNCTION(float3x4 (float3x4), test_exp_m34);
	JIT_FUNCTION(float3x4 (float3x4), test_exp2_m34);
	JIT_FUNCTION(float3x4 (float3x4), test_sin_m34);
	JIT_FUNCTION(float3x4 (float3x4), test_cos_m34);
	JIT_FUNCTION(float3x4 (float3x4), test_tan_m34);
	JIT_FUNCTION(float3x4 (float3x4), test_sinh_m34);
	JIT_FUNCTION(float3x4 (float3x4), test_cosh_m34);
	JIT_FUNCTION(float3x4 (float3x4), test_tanh_m34);
	JIT_FUNCTION(float3x4 (float3x4), test_asin_m34);
	JIT_FUNCTION(float3x4 (float3x4), test_acos_m34);
	JIT_FUNCTION(float3x4 (float3x4), test_atan_m34);
	JIT_FUNCTION(float3x4 (float3x4), test_ceil_m34);
	JIT_FUNCTION(float3x4 (float3x4), test_floor_m34);
	JIT_FUNCTION(float3x4 (float3x4), test_round_m34);
	JIT_FUNCTION(float3x4 (float3x4), test_trunc_m34);
	JIT_FUNCTION(float3x4 (float3x4), test_log_m34);
	JIT_FUNCTION(float3x4 (float3x4), test_log2_m34);
	JIT_FUNCTION(float3x4 (float3x4), test_log10_m34);
	JIT_FUNCTION(float3x4 (float3x4), test_rsqrt_m34);
	JIT_FUNCTION(float3x4 (float3x4, float3x4), test_ldexp_m34);
	JIT_FUNCTION(float (float), test_sqrt_f);
	JIT_FUNCTION(vec2 (vec2), test_sqrt_f2);
	JIT_FUNCTION(vec3 (vec3, vec3), test_cross_prod);
	JIT_FUNCTION(float (vec2, vec3, vec3), test_distance);
	JIT_FUNCTION(vec4 (vec3, float, vec3), test_fmod);
	JIT_FUNCTION(vec3 (vec3, vec3, vec3), test_lerp);
	JIT_FUNCTION(vec3 (vec3), test_rad_deg);
	JIT_FUNCTION(vec3 (vec3), test_norm_f3);
	JIT_FUNCTION(bool4(vec3, int3), test_any_all);
	JIT_FUNCTION(vec2 (vec2, vec4), test_length );
	JIT_FUNCTION(int3 (int3, int3, int3), test_clamp_i3);
	JIT_FUNCTION(int3 (int3, int3, int3), test_mad_i3);
	JIT_FUNCTION(int3 (int3, int3), test_min_i3);
	JIT_FUNCTION(int3 (int3, int3), test_max_i3);
	JIT_FUNCTION(float2x3 (float2x3, float2x3, float2x3), test_clamp_m23);
	JIT_FUNCTION(float2x3 (float2x3, float2x3, float2x3), test_mad_m23);
	JIT_FUNCTION(float2x3 (float2x3, float2x3), test_min_m23);
	JIT_FUNCTION(float2x3 (float2x3, float2x3), test_max_m23);
	JIT_FUNCTION(float2x3 (float2x3, float2x3), test_step_m23);
	JIT_FUNCTION(float2x3 (float2x3), test_saturate_m23 );
	JIT_FUNCTION(uint3 (uint3), test_countbits_u3);
	JIT_FUNCTION(uint3 (uint3), test_count_bits_u3);
	JIT_FUNCTION(int3 (int3),  test_firstbithigh_i3);
	JIT_FUNCTION(uint2(uint2), test_firstbithigh_u2);
	JIT_FUNCTION(int3 (int3),  test_firstbitlow_i3);
	JIT_FUNCTION(uint2(uint2), test_firstbitlow_u2);
	JIT_FUNCTION(int3 (int3),  test_reversebits_i3);
	JIT_FUNCTION(uint2(uint2), test_reversebits_u2);
	JIT_FUNCTION(bool3x3 (float3x3), test_isinf_m33);
	JIT_FUNCTION(bool3x3 (float3x3), test_isfinite_m33);
	JIT_FUNCTION(bool3x3 (float3x3), test_isnan_m33);
	JIT_FUNCTION(vec3 (vec3), test_frac_f3);
	JIT_FUNCTION(float3x4 (float3x4), test_rcp_m34);
	JIT_FUNCTION(float3x4 (float3x4, float3x4), test_pow_m34);
	JIT_FUNCTION(vec3 (vec3, vec3), test_reflect_f3);
	JIT_FUNCTION(vec3 (vec3, vec3, float), test_refract_f3);
	JIT_FUNCTION(int3x3 (float3x3), test_sign_m33);
	JIT_FUNCTION(float2x3 (float2x3, float2x3, float2x3), test_smoothstep_m23);
	JIT_FUNCTION(vec4 (float, float, float), test_lit);
	JIT_FUNCTION(vec3 (vec3, vec3, vec3), test_faceforward_f3);
	{
		vec3 lhs( 4.0f, 9.3f, -5.9f );
		vec3 rhs( 1.0f, -22.0f, 8.28f );
		vec3 ng(8.2f, 1.6f, 0.3f);

		float f = test_dot_f3(&lhs, &rhs);
		BOOST_CHECK_CLOSE( dot_prod3( lhs.xyz(), rhs.xyz() ), f, 0.0001 );
		vec3 ret0 = test_norm_f3(lhs);
		vec3 ret1 = test_norm_f3(rhs);
		vec3 ref0 = normalize3(lhs);
		vec3 ref1 = normalize3(rhs);

		vec3 ret2 = test_reflect_f3(lhs, rhs);
		vec3 ret3 = test_reflect_f3(rhs, lhs);
		vec3 ret4 = test_refract_f3(lhs, rhs, 0.22f);
		vec3 ret5 = test_refract_f3(lhs, rhs, 0.71f);
		vec3 ret6 = test_refract_f3(rhs, lhs, 0.22f);
		vec3 ret7 = test_refract_f3(rhs, lhs, 0.71f);

		vec3 ret8 = test_faceforward_f3(lhs, rhs, ng);
		vec3 ret9 = test_faceforward_f3(rhs, ng, lhs);
		vec3 ret10= test_faceforward_f3(rhs, lhs, ng);

		
		vec4 ret11= test_lit(lhs[0], lhs[1], lhs[2]);
		vec4 ret12= test_lit(rhs[0], rhs[1], rhs[2]);
		vec4 ret13= test_lit(ng [0], ng [1], ng [2]);

		vec3 ref2 = reflect3(lhs, rhs);
		vec3 ref3 = reflect3(rhs, lhs);
		vec3 ref4 = refract3(lhs, rhs, 0.22f);
		vec3 ref5 = refract3(lhs, rhs, 0.71f);
		vec3 ref6 = refract3(rhs, lhs, 0.22f);
		vec3 ref7 = refract3(rhs, lhs, 0.71f);
		vec3 ref8 = faceforward(lhs, rhs, ng);
		vec3 ref9 = faceforward(rhs, ng, lhs);
		vec3 ref10=	faceforward(rhs, lhs, ng);

		vec4 ref11= lit(lhs[0], lhs[1], lhs[2]);
		vec4 ref12= lit(rhs[0], rhs[1], rhs[2]);
		vec4 ref13= lit(ng [0], ng [1], ng [2]);

		for(int i = 0; i < 3; ++i)
		{
			BOOST_CHECK_CLOSE(ret0[i], ref0[i], 0.00001f);
			BOOST_CHECK_CLOSE(ret1[i], ref1[i], 0.00001f);
			BOOST_CHECK_CLOSE(ret2[i], ref2[i], 0.00001f);
			BOOST_CHECK_CLOSE(ret3[i], ref3[i], 0.00001f);
			BOOST_CHECK_CLOSE(ret4[i], ref4[i], 0.02000f);
			BOOST_CHECK_CLOSE(ret5[i], ref5[i], 0.02000f);
			BOOST_CHECK_CLOSE(ret6[i], ref6[i], 0.02000f);
			BOOST_CHECK_CLOSE(ret7[i], ref7[i], 0.02000f);
			BOOST_CHECK_CLOSE(ret8[i], ref8[i], 0.02000f);
			BOOST_CHECK_CLOSE(ret9[i], ref9[i], 0.02000f);
			BOOST_CHECK_CLOSE(ret10[i],ref10[i],0.02000f);
			BOOST_CHECK_CLOSE(ret11[i],ref11[i],0.00001f);
			BOOST_CHECK_CLOSE(ret12[i],ref12[i],0.00001f);
			BOOST_CHECK_CLOSE(ret13[i],ref13[i],0.00001f);
		}
	}
	{
		mat44 mat( mat44::identity() );
		mat.data_[0][0] = 1.0f;
		mat.data_[0][1] = 1.0f;
		mat.data_[0][2] = 1.0f;
		mat.data_[0][3] = 1.0f;

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

		BOOST_CHECK_CLOSE( f[0], refv[0], 0.00001f );
		BOOST_CHECK_CLOSE( f[1], refv[1], 0.00001f );
		BOOST_CHECK_CLOSE( f[2], refv[2], 0.00001f );
		BOOST_CHECK_CLOSE( f[3], refv[3], 0.00001f );
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
		float arr[12] =
		{
			17.7f, 66.3f, 0.92f,
			-88.7f, 8.6f, -0.22f,
			17.1f, -64.4f, 199.8f, 
			0.1f, -0.1f, 99.73f
		};

		float ref_v[12] = {0};

		for( int i = 0; i < 3; ++i )
		{
			for( int j = 0; j < 4; ++j )
			{
				ref_v[i*4+j] = (float)exp( (double)arr[i*4+j] );
			}
		}

		float3x4&  m34( reinterpret_cast<float3x4&>(arr) );
		
		float3x4 ret = test_exp_m34(m34);

		for( int i = 0; i < 3; ++i )
		{
			for( int j = 0; j < 4; ++j )
			{
				// Fix for NAN and INF
				if( *(int*)(&ref_v[i*4+j]) == *(int*)(&ret.data_[i][j]) )
				{
					BOOST_CHECK_BITWISE_EQUAL( *(int*)(&ref_v[i*4+j]), *(int*)(&ret.data_[i][j]) );
				}
				else
				{
					BOOST_CHECK_CLOSE( ref_v[i*4+j], ret.data_[i][j], 0.000001f );
				}
			}
		}
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
	{
		vec3 v0(227.5f, -0.33f, -76.4f);
		vec3 v1(-113.8f, 17.22f, -9.44f);
		vec2 v2(87.9f, 54.2f);
		float f = 15.5;

		vec4  dst(1.0f, v1[1]*v0[0], v1[2], v0[1]);
		float ref0 = ( v2-v1.xy() ).length() + ( v2.xyxy() - dst ).length();
		vec4  ref1 = vec4( fmodf(v0[0], v1[0]), fmodf(v0[1], v1[1]), fmodf(v0[2], v1[2]), fmodf(f, v1[1]) );
		vec3  ref2 = v0 + (v1-v0)*vec3(v2[0], v2[1], f);
		vec3  ref3 = ( (eflib::PI_FLOAT/180.0f)*v0.xy() ).xxy() + (180.0f/eflib::PI_FLOAT)*v0;
		vec2  ref4(v2.length(), v0.xyzy().length());

		float ret0 = test_distance(v2, v0, v1);
		vec4  ret1 = test_fmod(v0, f, v1);
		vec3  ret2 = test_lerp( v0, v1, vec3(v2[0], v2[1], f) );
		vec3  ret3 = test_rad_deg(v0);
		vec2  ret4 = test_length( v2, v0.xyzy() );

		BOOST_CHECK_CLOSE( ret0,	ref0,	 0.000001f );
		BOOST_CHECK_CLOSE( ret1[0], ref1[0], 0.000001f );
		BOOST_CHECK_CLOSE( ret1[1], ref1[1], 0.000001f );
		BOOST_CHECK_CLOSE( ret1[2], ref1[2], 0.000001f );
		BOOST_CHECK_CLOSE( ret1[3], ref1[3], 0.000001f );
		BOOST_CHECK_CLOSE( ret2[0], ref2[0], 0.00001f );
		BOOST_CHECK_CLOSE( ret2[1], ref2[1], 0.00001f );
		BOOST_CHECK_CLOSE( ret2[2], ref2[2], 0.00001f );
		BOOST_CHECK_CLOSE( ret3[0], ref3[0], 0.00001f );
		BOOST_CHECK_CLOSE( ret3[1], ref3[1], 0.00001f );
		BOOST_CHECK_CLOSE( ret3[2], ref3[2], 0.000001f );
		BOOST_CHECK_CLOSE( ret4[0], ref4[0], 0.00001f );
		BOOST_CHECK_CLOSE( ret4[1], ref4[1], 0.00001f );
	}
	{
		vec3 v0( 0.0f,  0.0f,  0.0f );
		vec3 v1( 2.3f, -1.7f,  0.0f );
		vec3 v2( 2.3f, -1.7f,  7.7f );

		int3 i0(  0,  0,  0 );
		int3 i1( 15,  0, -7 );
		int3 i2( 10, -9, 11 );

		bool4 ret0 = test_any_all(v0, i0);
		bool4 ret1 = test_any_all(v1, i1);
		bool4 ret2 = test_any_all(v2, i2);

		BOOST_CHECK_EQUAL( ret0[0], false );
		BOOST_CHECK_EQUAL( ret0[1], false );
		BOOST_CHECK_EQUAL( ret0[2], false );
		BOOST_CHECK_EQUAL( ret0[3], false );

		BOOST_CHECK_EQUAL( ret1[0], true  );
		BOOST_CHECK_EQUAL( ret1[1], false );
		BOOST_CHECK_EQUAL( ret1[2], true );
		BOOST_CHECK_EQUAL( ret1[3], false );

		BOOST_CHECK_EQUAL( ret2[0], true );
		BOOST_CHECK_EQUAL( ret2[1], true );
		BOOST_CHECK_EQUAL( ret2[2], true );
		BOOST_CHECK_EQUAL( ret2[3], true );
	}
	{
		float lhs_array[3][4] =
		{
			{17.7f, 66.3f, 0.92f, -88.7f},
			{8.6f, -0.22f, 17.1f, -64.4f},
			{199.8f, 0.1f, -0.1f, 99.73f}
		};

		float rhs_array[3][4] =
		{
			{9.62f, 10.33f, -18.2f, 99.7f},
			{-0.3f, -76.9f, 93.3f,  0.22f},
			{44.1f, 0.027f, 19.9f, -33.5f}
		};

		float3x4& lhs( reinterpret_cast<float3x4&>(lhs_array) );
		float3x4& rhs( reinterpret_cast<float3x4&>(rhs_array) );

		float3x4 ret_exp2	= test_exp2_m34(lhs);
		float3x4 ret_sin	= test_sin_m34(lhs);
		float3x4 ret_cos	= test_cos_m34(lhs);
		float3x4 ret_tan	= test_tan_m34(lhs);
		float3x4 ret_sinh	= test_sinh_m34(lhs);
		float3x4 ret_cosh	= test_cosh_m34(lhs);
		float3x4 ret_tanh	= test_tanh_m34(lhs);
		float3x4 ret_asin	= test_asin_m34(lhs);
		float3x4 ret_acos	= test_acos_m34(lhs);
		float3x4 ret_atan	= test_atan_m34(lhs);
		float3x4 ret_ceil	= test_ceil_m34(lhs);
		float3x4 ret_floor	= test_floor_m34(lhs);
		float3x4 ret_round	= test_round_m34(lhs);
		float3x4 ret_trunc	= test_trunc_m34(lhs);
		float3x4 ret_log	= test_log_m34(lhs);
		float3x4 ret_log2	= test_log2_m34(lhs);
		float3x4 ret_log10	= test_log10_m34(lhs);
		float3x4 ret_rsqrt	= test_rsqrt_m34(lhs);
		float3x4 ret_ldexp	= test_ldexp_m34(lhs,rhs);
		float3x4 ret_rcp	= test_rcp_m34(lhs);
		float3x4 ret_pow	= test_pow_m34(lhs, rhs);

		for( int i = 0; i < 3; ++i ) {
			for( int j = 0; j < 4; ++j ) {
				
				union { float f; uint32_t u; } ret, ref;

				ret.f = ret_exp2.data_[i][j];	ref.f = ldexp(1.0f, lhs_array[i][j]);	BOOST_CHECK_BITWISE_EQUAL( ret.u, ref.u );
				ret.f = ret_sin.data_[i][j];	ref.f = sinf(lhs_array[i][j]);			BOOST_CHECK_BITWISE_EQUAL( ret.u, ref.u );
				ret.f = ret_cos.data_[i][j];	ref.f = cosf(lhs_array[i][j]);			BOOST_CHECK_BITWISE_EQUAL( ret.u, ref.u );
				ret.f = ret_tan.data_[i][j];	ref.f = tanf(lhs_array[i][j]);			BOOST_CHECK_BITWISE_EQUAL( ret.u, ref.u );
				ret.f = ret_sinh.data_[i][j];	ref.f = sinhf(lhs_array[i][j]);			BOOST_CHECK_BITWISE_EQUAL( ret.u, ref.u );
				ret.f = ret_cosh.data_[i][j];	ref.f = coshf(lhs_array[i][j]);			BOOST_CHECK_BITWISE_EQUAL( ret.u, ref.u );
				ret.f = ret_tanh.data_[i][j];	ref.f = tanhf(lhs_array[i][j]);			BOOST_CHECK_BITWISE_EQUAL( ret.u, ref.u );
				ret.f = ret_asin.data_[i][j];	ref.f = asinf(lhs_array[i][j]);			BOOST_CHECK_BITWISE_EQUAL( ret.u, ref.u );
				ret.f = ret_acos.data_[i][j];	ref.f = acosf(lhs_array[i][j]);			BOOST_CHECK_BITWISE_EQUAL( ret.u, ref.u );
				ret.f = ret_atan.data_[i][j];	ref.f = atanf(lhs_array[i][j]);			BOOST_CHECK_BITWISE_EQUAL( ret.u, ref.u );
				ret.f = ret_ceil.data_[i][j];	ref.f = fast_ceil(lhs_array[i][j]);		BOOST_CHECK_BITWISE_EQUAL( ret.u, ref.u );
				ret.f = ret_floor.data_[i][j];	ref.f = fast_floor(lhs_array[i][j]);	BOOST_CHECK_BITWISE_EQUAL( ret.u, ref.u );
				ret.f = ret_round.data_[i][j];	ref.f = fast_round(lhs_array[i][j]);	BOOST_CHECK_BITWISE_EQUAL( ret.u, ref.u );
				ret.f = ret_trunc.data_[i][j];	ref.f = trunc(lhs_array[i][j]);			BOOST_CHECK_BITWISE_EQUAL( ret.u, ref.u );
				ret.f = ret_log.data_[i][j];	ref.f = fast_log(lhs_array[i][j]);		BOOST_CHECK_BITWISE_EQUAL( ret.u, ref.u );
				ret.f = ret_log2.data_[i][j];	ref.f = fast_log2(lhs_array[i][j]);		BOOST_CHECK_BITWISE_EQUAL( ret.u, ref.u );
				ret.f = ret_log10.data_[i][j];	ref.f = log10f(lhs_array[i][j]);		BOOST_CHECK_BITWISE_EQUAL( ret.u, ref.u );
				ret.f = ret_rsqrt.data_[i][j];	ref.f = 1.0f/sqrtf(lhs_array[i][j]);	BOOST_CHECK_BITWISE_EQUAL( ret.u, ref.u );
				ret.f = ret_rcp.data_[i][j];	ref.f = 1.0f / lhs_array[i][j];			BOOST_CHECK_CLOSE( ret.f, ref.f, 0.00001f );
				ret.f = ret_ldexp.data_[i][j];	ref.f = ldexpf(lhs_array[i][j], rhs_array[i][j]);	BOOST_CHECK_BITWISE_EQUAL( ret.u, ref.u );
				ret.f = ret_pow.data_[i][j];	ref.f = powf(lhs_array[i][j], rhs_array[i][j]);		BOOST_CHECK_BITWISE_EQUAL( ret.u, ref.u );
			}
		}
	}	
	{
		int3 min_v( 98, -76, 0 );
		int3 max_v( 226, 19, 0 );
		int3 v0   ( -92, 61, 37);
		int3 v1   ( 227,-26, -3);

		int3 ret_v0 = test_clamp_i3( v0, min_v, max_v );
		int3 ret_v1 = test_clamp_i3( v1, min_v, max_v );
		int3 ret_v2 = test_min_i3(v0, max_v);
		int3 ret_v3 = test_max_i3(v1, min_v);
		int3 ret_v4 = test_mad_i3(v0, v1, max_v);

		BOOST_CHECK_EQUAL( ret_v0[0], min_v[0] );
		BOOST_CHECK_EQUAL( ret_v0[1], max_v[1] );
		BOOST_CHECK_EQUAL( ret_v0[2], max_v[2] );

		BOOST_CHECK_EQUAL( ret_v1[0], max_v[0] );
		BOOST_CHECK_EQUAL( ret_v1[1], v1   [1] );
		BOOST_CHECK_EQUAL( ret_v1[2], max_v[2] );

		BOOST_CHECK_EQUAL( ret_v2[0], std::min(v0[0], max_v[0]) );
		BOOST_CHECK_EQUAL( ret_v2[1], std::min(v0[1], max_v[1]) );
		BOOST_CHECK_EQUAL( ret_v2[2], std::min(v0[2], max_v[2]) );

		BOOST_CHECK_EQUAL( ret_v3[0], std::max(v1[0], min_v[0]) );
		BOOST_CHECK_EQUAL( ret_v3[1], std::max(v1[1], min_v[1]) );
		BOOST_CHECK_EQUAL( ret_v3[2], std::max(v1[2], min_v[2]) );

		BOOST_CHECK_EQUAL( ret_v4[0], v0[0]*v1[0]+max_v[0] );
		BOOST_CHECK_EQUAL( ret_v4[1], v0[1]*v1[1]+max_v[1] );
		BOOST_CHECK_EQUAL( ret_v4[2], v0[2]*v1[2]+max_v[2] );

		float m[2][3] =
		{
			{17.7f, 66.3f, 0.92f},
			{-88.7f, 8.6f,-0.22f}
		};

		float min_m[2][3] =
		{
			{17.1f,-64.4f,199.8f},
			{0.1f, -0.1f, 99.73f}
		};

		float max_m[2][3] =
		{
			{9.62f, 10.33f, -18.2f},
			{99.7f,  -0.3f, -76.9f}
		};

		for(int i = 0; i < 2; ++i)
		{
			for(int j = 0; j < 3; ++j)
			{
				if (min_m[i][j] > max_m[i][j])
				{
					std::swap(min_m[i][j], max_m[i][j]);
				}
			}
		}
		float2x3 ret0 = test_clamp_m23(
			reinterpret_cast<float2x3&>(m),
			reinterpret_cast<float2x3&>(min_m),
			reinterpret_cast<float2x3&>(max_m)
			);
		float2x3 ret1 = test_min_m23(
			reinterpret_cast<float2x3&>(m),
			reinterpret_cast<float2x3&>(max_m)
			);
		float2x3 ret2 = test_max_m23(
			reinterpret_cast<float2x3&>(m),
			reinterpret_cast<float2x3&>(max_m)
			);
		float2x3 ret3 = test_saturate_m23( reinterpret_cast<float2x3&>(m) );
		float2x3 ret4 = test_mad_m23(
			reinterpret_cast<float2x3&>(max_m),
			reinterpret_cast<float2x3&>(min_m),
			reinterpret_cast<float2x3&>(m)
			);
		float2x3 ret5 = test_step_m23(
			reinterpret_cast<float2x3&>(min_m),
			reinterpret_cast<float2x3&>(m)
			);
		float2x3 ret6 = test_smoothstep_m23(
			reinterpret_cast<float2x3&>(min_m),
			reinterpret_cast<float2x3&>(max_m),
			reinterpret_cast<float2x3&>(m)
			);

		for(int i = 0; i < 2; ++i)
		{
			for(int j = 0; j < 3; ++j)
			{
				BOOST_CHECK_EQUAL(
					ret0.data_[i][j],
					clamp( m[i][j], min_m[i][j], max_m[i][j] ) 
					);
				BOOST_CHECK_EQUAL(
					ret1.data_[i][j],
					std::min(m[i][j], max_m[i][j])
					);
				BOOST_CHECK_EQUAL(
					ret2.data_[i][j],
					std::max(m[i][j], max_m[i][j])
					);
				BOOST_CHECK_EQUAL(
					ret3.data_[i][j],
					clamp(m[i][j], 0.0f, 1.0f)
					);
				BOOST_CHECK_EQUAL(
					ret4.data_[i][j],
					max_m[i][j]*min_m[i][j]+m[i][j]
					);
				BOOST_CHECK_EQUAL(
					ret5.data_[i][j],
					min_m[i][j]<=m[i][j]?1.0f:0.0f
					);
				BOOST_CHECK_CLOSE(
					ret6.data_[i][j],
					smoothstep(min_m[i][j], max_m[i][j], m[i][j]),
					0.0002f
					);
			}
		}
	}
	{
		uint3 v0(0, 0xFFFFFFFF, 7895567);
		uint3 v1(678287, 99271, 7);

		uint3 ret0 = test_countbits_u3(v0);
		uint3 ret1 = test_count_bits_u3(v1);
		
		BOOST_CHECK_EQUAL( ret0[0], 0 );
		BOOST_CHECK_EQUAL( ret0[1], 32 );
		BOOST_CHECK_EQUAL( ret0[2], count_bits(v0[2]) );

		BOOST_CHECK_EQUAL( ret1[0], count_bits(v1[0]) );
		BOOST_CHECK_EQUAL( ret1[1], count_bits(v1[1]) );
		BOOST_CHECK_EQUAL( ret1[2], count_bits(v1[2]) );
	}
	{
		int3  v0( static_cast<int>(0x7F276898), 0xF0, -1 );
		uint2 v1( 0x7F276898, 0xF0 );

		int3  ret0 = test_firstbithigh_i3(v0);
		int3  ret1 = test_firstbitlow_i3 (v0);
		
		uint2 ret2 = test_firstbithigh_u2(v1);
		uint2 ret3 = test_firstbitlow_u2 (v1);

		int3  ret4 = test_reversebits_i3(v0);
		uint2 ret5 = test_reversebits_u2(v1);
		
		int32_t  ref4[3] = {
			static_cast<int32_t>( naive_reversebits( static_cast<uint32_t>(v0[0]) ) ),
			static_cast<int32_t>( naive_reversebits( static_cast<uint32_t>(v0[1]) ) ),
			static_cast<int32_t>( naive_reversebits( static_cast<uint32_t>(v0[2]) ) )
		};

		uint32_t ref5[2] = {
			naive_reversebits(v1[0]),
			naive_reversebits(v1[1])
		};

		BOOST_CHECK_EQUAL(ret0[0], 1);
		BOOST_CHECK_EQUAL(ret0[1], 24);
		BOOST_CHECK_EQUAL(ret0[2], 0);

		BOOST_CHECK_EQUAL(ret1[0], 3);
		BOOST_CHECK_EQUAL(ret1[1], 4);
		BOOST_CHECK_EQUAL(ret1[2], 0);

		BOOST_CHECK_EQUAL(ret2[0], 1);
		BOOST_CHECK_EQUAL(ret2[1], 24);

		BOOST_CHECK_EQUAL(ret3[0], 3);
		BOOST_CHECK_EQUAL(ret3[1], 4);

		BOOST_CHECK_EQUAL(ret4[0], ref4[0]);
		BOOST_CHECK_EQUAL(ret4[1], ref4[1]);
		BOOST_CHECK_EQUAL(ret4[2], ref4[2]);
		BOOST_CHECK_EQUAL(ret5[0], ref5[0]);
		BOOST_CHECK_EQUAL(ret5[1], ref5[1]);
	}
	{
		float v[3][3] = 
		{
			{ 0.0f,  numeric_limits<float>::quiet_NaN(), 75.5f },
			{ -886.8f, numeric_limits<float>::infinity(), numeric_limits<float>::signaling_NaN() },
			{ -numeric_limits<float>::infinity(), -0.0f, numeric_limits<float>::max() }
		};

		bool3x3 ret_inf = test_isinf_m33( reinterpret_cast<float3x3&>(v) );
		bool3x3 ret_nan = test_isnan_m33( reinterpret_cast<float3x3&>(v) );
		bool3x3 ret_fin = test_isfinite_m33( reinterpret_cast<float3x3&>(v) );
		int3x3	ret_sgn = test_sign_m33( reinterpret_cast<float3x3&>(v) );
		for(int i = 0; i < 3; ++i)
		{
			for(int j = 0; j < 3; ++j)
			{
				BOOST_CHECK_EQUAL( boost::math::isnan(v[i][j]),		ret_nan.data_[i][j] != 0 );
				BOOST_CHECK_EQUAL( boost::math::isinf(v[i][j]),		ret_inf.data_[i][j] != 0 );
				BOOST_CHECK_EQUAL( boost::math::isfinite(v[i][j]),	ret_fin.data_[i][j] != 0 );
				BOOST_CHECK_EQUAL( v[i][j] > 0.0f ? 1 : (v[i][j] < 0.0f ? -1 : 0), ret_sgn.data_[i][j] );
			}
		}
	}
	{
		vec3 v0(-2.0f, 1.7f, 0.0f);
		vec3 v1(-2.11f, 22.05f, 3.8f);

		vec3 ret0 = test_frac_f3(v0);
		vec3 ret1 = test_frac_f3(v1);

		BOOST_CHECK_CLOSE(ret0[0], 0.0f, (0.00001f*fabsf(v0[0])/1.0f) );
		BOOST_CHECK_CLOSE(ret0[1], 0.7f, (0.00001f*fabsf(v0[1])/0.7f) );
		BOOST_CHECK_CLOSE(ret0[2], 0.0f, (0.00001f*fabsf(v0[2])/1.0f) );

		BOOST_CHECK_CLOSE(ret1[0], 0.11f, 0.00001f*fabsf(v1[0])/0.11f);
		BOOST_CHECK_CLOSE(ret1[1], 0.05f, 0.00001f*fabsf(v1[1])/0.05f);
		BOOST_CHECK_CLOSE(ret1[2], 0.8f , 0.00001f*fabsf(v1[2])/0.8f );
	}
#endif
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
	mat_mul( mat, mat_rotX(tmpMat, 0.2f), mat );
	mat_mul( mat, mat_rotY(tmpMat, -0.3f), mat );
	mat_mul( mat, mat_translate(tmpMat, 1.7f, -0.9f, 1.1f), mat );
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
	BOOST_CHECK_CLOSE( bout.pos[0], out_pos[0], 0.0001f );
	BOOST_CHECK_CLOSE( bout.pos[1], out_pos[1], 0.0001f );
	BOOST_CHECK_CLOSE( bout.pos[2], out_pos[2], 0.0001f );
	BOOST_CHECK_CLOSE( bout.pos[3], out_pos[3], 0.0001f );
}

#endif

#if ALL_TESTS_ENABLED

struct tex2d_vs_data{
	vec4 pos;
};

struct tex2d_vs_bout{
	vec4 pos;
};

struct tex2d_vs_sin{
	vec4 *position;
};

struct tex2d_vs_bin{
	static int ph;
	void* s;
};

void tex2Dlod_vs(vec4* out, void* s, vec4* in)
{
	BOOST_CHECK_EQUAL(in->data_[0], 3.0f);
	BOOST_CHECK_EQUAL(in->data_[1], 4.5f);
	BOOST_CHECK_EQUAL(in->data_[2], 2.6f);
	BOOST_CHECK_EQUAL(in->data_[3], 1.0f);

	BOOST_CHECK_EQUAL(s, &tex2d_vs_bin::ph);

	out->data_[0] = 9.3f;
	out->data_[1] = 8.7f;
	out->data_[2] = -29.6f;
	out->data_[3] = 0.0f;
}
int tex2d_vs_bin::ph = 335;

BOOST_FIXTURE_TEST_CASE( tex_vs, jit_fixture ){
	init_vs("./repo/question/v1a1/tex.svs");

	set_raw_function( &tex2Dlod_vs, "sasl.vs.tex2d.lod" );

	vec4 pos(3.0f, 4.5f, 2.6f, 1.0f);

	tex2d_vs_data data;
	data.pos = pos;

	tex2d_vs_sin sin;
	sin.position = &data.pos;

	tex2d_vs_bin bin;
	bin.s = &tex2d_vs_bin::ph;
	
	tex2d_vs_bout bout;

	JIT_FUNCTION( void(tex2d_vs_sin*, tex2d_vs_bin*, void*, tex2d_vs_bout*), fn );
	
	fn(&sin, &bin, (void*)NULL, &bout);

	vec4 out_pos(9.3f, 8.7f, -29.6f, 0.0f);

	BOOST_CHECK_EQUAL( bout.pos[0], out_pos[0] );
	BOOST_CHECK_EQUAL( bout.pos[1], out_pos[1] );
	BOOST_CHECK_EQUAL( bout.pos[2], out_pos[2] );
	BOOST_CHECK_EQUAL( bout.pos[3], out_pos[3] );
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
	jit_function<char3(int3, int3, int3)> test_vbool;
	jit_function<bool3x4 (float3x4, float3x4, float3x4)> test_mbool;

	function( test_max, "test_max" );
	function( test_min, "test_min" );
	function( test_le, "test_le" );
	function( test_ge, "test_ge" );
	function( test_short, "test_short" );
	function( test_vbool, "test_vbool" );
	function( test_mbool, "test_mbool" );

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

	{
		for( int i = -1; i < 2; ++i ){
			for( int j = -1; j < 2; ++j ){
				for( int k = -1; k < 2; ++k ){
					bool short_result = test_short(i, j, k);
					bool ref_result = test_short_ref(i, j, k);
					BOOST_CHECK_EQUAL( short_result, ref_result );
				}
			}
		}
		int3 x(87,  46, -22);
		int3 y(65, -11,   9);
		int3 z(37,  16,  22);

		bool ref_v[3];
		ref_v[0] = x[0] > y[0] || x[0] > z[0] && x[0] <= y[0]+z[0];  
		ref_v[1] = x[1] > y[1] || x[1] > z[1] && x[1] <= y[1]+z[1];  
		ref_v[2] = x[2] > y[2] || x[2] > z[2] && x[2] <= y[2]+z[2];  

		char3 ret_v = test_vbool(x, y, z);

		BOOST_CHECK_EQUAL( static_cast<bool>(ret_v.data_[0]), ref_v[0] );
		BOOST_CHECK_EQUAL( static_cast<bool>(ret_v.data_[1]), ref_v[1] );
		BOOST_CHECK_EQUAL( static_cast<bool>(ret_v.data_[2]), ref_v[2] );
	}

	{
		float array0[3][4] =
		{
			{17.7f, 66.3f, 0.92f, -88.7f},
			{8.6f, -0.22f, 17.1f, -64.4f},
			{199.8f, 0.1f, -0.1f, 99.73f}
		};

		float array1[3][4] =
		{
			{9.62f, 10.33f, -18.2f, 99.7f},
			{-0.3f, -76.9f, 93.3f,  0.22f},
			{44.1f, 0.027f, 19.9f, -33.5f}
		};

		float array2[3][4] = 
		{
			{0.198f, 10.3f, -0.82f, 9.37f},
			{7.3f, -7.92f, 93.3f,  10.22f},
			{24.1f, -0.77f, -40.9f, 43.5f}
		};

		char ref_v[3][4] = {false};
		for( int i = 0; i < 3; ++i )
		{
			for( int j = 0; j < 4; ++j )
			{
				// i > j || i > k && i <= j+k;  

				ref_v[i][j] 
				=  array0[i][j] > array1[i][j]
				|| array0[i][j] > array2[i][j]
				&& array0[i][j] < (array1[i][j] + array2[i][j])
				;
			}
		}

		float3x4& m0( reinterpret_cast<float3x4&>(array0) );
		float3x4& m1( reinterpret_cast<float3x4&>(array1) );
		float3x4& m2( reinterpret_cast<float3x4&>(array2) );

		bool3x4 ret = test_mbool( m0, m1, m2 );

		for( int i = 0; i < 3; ++i )
		{
			for( int j = 0; j < 4; ++j )
			{
				BOOST_CHECK_EQUAL( ref_v[i][j], ret.data_[i][j] == 1 );
			}
		}
	}
}
#endif

#if ALL_TESTS_ENABLED
BOOST_FIXTURE_TEST_CASE( unary_operators_test, jit_fixture )
{
	init_g( "./repo/question/v1a1/unary_operators.ss" );

	jit_function<int(int)> test_pre_inc, test_pre_dec, test_post_inc, test_post_dec;
	jit_function<int4(int3,int)>		test_neg_i;
	jit_function<float3x4(float3x4)>	test_neg_f;
	jit_function<bool2x3(bool2x3)>		test_not;
	jit_function<uint2x3(uint2x3)>		test_bit_not;

	function( test_pre_inc, "test_pre_inc" );
	BOOST_REQUIRE(test_pre_inc);
	function( test_pre_dec, "test_pre_dec" );
	BOOST_REQUIRE(test_pre_dec);
	function( test_post_inc, "test_post_inc" );
	BOOST_REQUIRE(test_post_inc);
	function( test_post_dec, "test_post_dec" );
	BOOST_REQUIRE(test_post_dec);
	function( test_neg_i, "test_neg_i" );
	BOOST_REQUIRE(test_neg_i);
	function( test_neg_f, "test_neg_f" );
	BOOST_REQUIRE(test_neg_f);
	function( test_not, "test_not" );
	BOOST_REQUIRE(test_not);
	function( test_bit_not, "test_bit_not" );
	BOOST_REQUIRE(test_bit_not);

	BOOST_CHECK( test_pre_inc(5) == 13 );
	BOOST_CHECK( test_pre_dec(5) == 7 );
	BOOST_CHECK( test_post_inc(5) == 11 );
	BOOST_CHECK( test_post_dec(5) == 9 );

	{
		int x[3] = { 0, 227, -876 };
		int y = 5;
		int4 ret = test_neg_i( reinterpret_cast<int3&>(x), y );
		BOOST_CHECK_EQUAL( ret[0], -x[0] );
		BOOST_CHECK_EQUAL( ret[1], -x[1] );
		BOOST_CHECK_EQUAL( ret[2], -x[2] );
		BOOST_CHECK_EQUAL( ret[3], -y    );
	}

	{
		float arr[3][4] =
		{
			{17.7f, 66.3f, 0.92f, -88.7f},
			{8.6f, -0.22f, 17.1f, -64.4f},
			{199.8f, 0.1f, -0.1f, 99.73f}
		};
		float3x4 ret = test_neg_f( reinterpret_cast<float3x4&>(arr) );
		for( int i = 0; i < 3; ++i )
		{
			for( int j = 0; j < 4; ++j )
			{
				BOOST_CHECK_CLOSE( -arr[i][j], ret.data_[i][j], 0.000001f );
			}
		}
	}

	{
		char arr[2][3] = 
		{
			{ 0, 1, 0 },
			{ 0, 1, 1 },
		};
		bool2x3 ret = test_not( reinterpret_cast<bool2x3&>(arr) );
		for( int i = 0; i < 2; ++i )
		{
			for( int j = 0; j < 3; ++j )
			{
				BOOST_CHECK_EQUAL( arr[i][j] == 0, ret.data_[i][j] == 1 );
			}
		}
	}

	{
		uint32_t arr[2][3] =
		{
			{ 786, 0, 33769097 },
			{ 0xFFFFFFFF, 3899927, 67}
		};
		uint2x3 ret = test_bit_not( reinterpret_cast<uint2x3&>(arr) );
		for( int i = 0; i < 2; ++i )
		{
			for( int j = 0; j < 3; ++j )
			{
				BOOST_CHECK_EQUAL( ~arr[i][j], ret.data_[i][j] );
			}
		}
	}
}
#endif

#if ALL_TESTS_ENABLED

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

	JIT_FUNCTION( int(int),					test_implicit_cast_i32_b );
	JIT_FUNCTION( float(int),				test_implicit_cast_i32_f32 );
	JIT_FUNCTION( int(float),				test_implicit_cast_f32_b );
	JIT_FUNCTION( float(int, float),		test_op_add_cast );
	JIT_FUNCTION( int(uint8_t,int),			test_op_sub_cast );
	JIT_FUNCTION( float(int),				test_sqrt_cast );
	JIT_FUNCTION( float(int2),				test_imp_v1_s_cast );
	JIT_FUNCTION( int2(vec2,uint2),			test_bitcast_to_i );
	JIT_FUNCTION( uint3(vec3,int3),			test_bitcast_to_u );
	JIT_FUNCTION( float(uint32_t,int32_t),	test_bitcast_to_f );
	JIT_FUNCTION( int2x3(float2x3,uint2x3),	test_bitcast_to_mi );
	JIT_FUNCTION( float2x3(int2x3),			test_mat_i2f );
	JIT_FUNCTION( int2x3(float2x3),			test_explicit_cast_f2i );
	JIT_FUNCTION( uint2x3(float2x3),		test_explicit_cast_f2u );

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
	BOOST_CHECK_EQUAL( test_op_sub_cast(-122,  8645), ((uint8_t)(-122))-8645 );
	BOOST_CHECK_EQUAL( test_op_sub_cast( 122, -8645),  122+8645 );

	BOOST_CHECK_CLOSE( test_sqrt_cast(0),	  0.0f,			   0.000001f );
	BOOST_CHECK_CLOSE( test_sqrt_cast(17652), sqrtf(17652.0f), 0.000001f );

	{
		int2 xy(86, 99);
		BOOST_CHECK_CLOSE( (float)86, test_imp_v1_s_cast(xy), 0.000001f );
	}

	{
		uint2 u2;
		u2[0] = 0xCCCCCCCC;
		u2[1] = 0x12345678;

		int3 i3(-87, 99, 0x7F836798);
		vec3 v3(-98.765f, 0.00765f, 198760000000.0f);
		vec2 v2(998.65f, -0.000287f);

		int2  to_i_ref_v = reinterpret_cast<int2&>(u2) + reinterpret_cast<int2&>(v2);
		uint3 to_u_ref_v = reinterpret_cast<uint3&>(v3) + reinterpret_cast<uint3&>(i3);
		float to_f_ref_v = reinterpret_cast<float&>(u2[0]) + reinterpret_cast<float&>(i3[0]);

		int2  to_i_ret_v = test_bitcast_to_i(v2, u2);
		uint3 to_u_ret_v = test_bitcast_to_u(v3, i3);
		float to_f_ret_v = test_bitcast_to_f(u2[0], i3[0]);

		BOOST_CHECK_BITWISE_EQUAL( to_i_ref_v[0], to_i_ret_v[0] );
		BOOST_CHECK_BITWISE_EQUAL( to_i_ref_v[1], to_i_ret_v[1] );

		BOOST_CHECK_BITWISE_EQUAL( to_u_ref_v[0], to_u_ret_v[0] );
		BOOST_CHECK_BITWISE_EQUAL( to_u_ref_v[1], to_u_ret_v[1] );

		BOOST_CHECK_BITWISE_EQUAL( *(uint32_t*)(&to_f_ref_v), *(uint32_t*)(&to_f_ret_v) );
	}

	{
		uint32_t uarr[2][3] =
		{
			{ 786, 0, 33769097 },
			{ 0xFFFFFFFF, 3899927, 67}
		};

		float farr[2][3] =
		{
			{17.7f, 0.92f, -88.7f},
			{8.6f, -0.22f, -64.4f}
		};

		int2x3 ret = test_bitcast_to_mi( reinterpret_cast<float2x3&>(farr), reinterpret_cast<uint2x3&>(uarr) );
		
		for(int i = 0; i < 2; ++i)
		{
			for(int j = 0; j < 3; ++j)
			{
				union {float f; int i;}		f2i;
				union {uint32_t u; int i;}	u2i;
				f2i.f = farr[i][j];
				u2i.u = uarr[i][j];
				int v = f2i.i + u2i.i;
				BOOST_CHECK_EQUAL( v, ret.data_[i][j] );
			}
		}
	}

	{
		int m23[2][3] =
		{
			{17, -82999738, -89287},
			{0, 97532589, 9870}
		};

		float2x3	ret0 = test_mat_i2f( reinterpret_cast<int2x3&>(m23) );
		int2x3		ret1 = test_explicit_cast_f2i(ret0);
		uint2x3		ret2 = test_explicit_cast_f2u(ret0);

		for(int i = 0; i < 2; ++i)
		{
			for(int j = 0; j < 3; ++j)
			{
				BOOST_CHECK_EQUAL( static_cast<float>(m23[i][j]),			ret0.data_[i][j] );
				BOOST_CHECK_EQUAL( static_cast<int>(ret0.data_[i][j]),		ret1.data_[i][j] );
				BOOST_CHECK_EQUAL( static_cast<uint32_t>(ret0.data_[i][j]),	ret2.data_[i][j] );
			}
		}
	}
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

	float* src[16] = {NULL};
	float* dst[16] = {NULL};
	float* src_data = (float*)_aligned_malloc( PACKAGE_ELEMENT_COUNT * sizeof(float), SIMD_ALIGNMENT );
	float* dst_data = (float*)_aligned_malloc( PACKAGE_ELEMENT_COUNT * sizeof(float), SIMD_ALIGNMENT );
	float dest_ref[PACKAGE_ELEMENT_COUNT];

	for( size_t i = 0; i < PACKAGE_ELEMENT_COUNT; ++i ){
		src[i] = src_data + i;
		dst[i] = dst_data + i;
		src_data[i] = (i+3.77f)*(0.76f*i);

		dest_ref[i] = src_data[i] + 5.0f;
		dest_ref[i] += (src_data[i] - 5.0f);
		dest_ref[i] += (src_data[i] * 5.0f);
	}

	fn( src, (void*)NULL, dst, (void*)NULL );
	
	for( size_t i = 0; i < PACKAGE_ELEMENT_COUNT; ++i ){
		BOOST_CHECK_CLOSE( dest_ref[i], *dst[i], 0.000012f );
	}

	_aligned_free( src_data );
	_aligned_free( dst_data );
}
#endif

#if ALL_TESTS_ENABLED
BOOST_FIXTURE_TEST_CASE(swizzle, jit_fixture)
{
	init_g("./repo/question/v1a1/swizzle.ss");

	jit_function<int3 (int4 x, int2 y)> fn;
	function(fn, "fn");

	int4 x(1, 2, 3, 4);
	int2 y(5, 6);

	int3 ref_v;
	ref_v[0] = x[2] + y[1]; 
	ref_v[1] = x[3] + y[0];
	ref_v[2] = x[3] + y[1];
	int3 tmp = int3( ref_v[2], ref_v[1], ref_v[0] );
	ref_v = tmp;

	int3 ret_v;
	ret_v = fn(x, y);

	BOOST_CHECK_EQUAL( ref_v[0], ret_v[0] );
	BOOST_CHECK_EQUAL( ref_v[1], ret_v[1] );
	BOOST_CHECK_EQUAL( ref_v[2], ret_v[2] );
}
#endif

#if ALL_TESTS_ENABLED
BOOST_FIXTURE_TEST_CASE( ps_swz_and_wm, jit_fixture )
{
	init_ps( "./repo/question/v1a1/swizzle_and_wm.sps" );

	jit_function<void(void*, void*, void*, void*)> fn;
	function( fn, "fn" );

	vec4* src[PACKAGE_ELEMENT_COUNT] = {NULL};
	vec4* dst[PACKAGE_ELEMENT_COUNT] = {NULL};
	vec4* src_data = (vec4*)_aligned_malloc( PACKAGE_ELEMENT_COUNT * sizeof(vec4), SIMD_ALIGNMENT );
	vec4* dst_data = (vec4*)_aligned_malloc( PACKAGE_ELEMENT_COUNT * sizeof(vec4), SIMD_ALIGNMENT );
	vec4 dest_ref[PACKAGE_ELEMENT_COUNT];

	for( size_t i = 0; i < PACKAGE_ELEMENT_COUNT * 4; ++i ){
		((float*)src_data)[i] = (i+3.77f)*(0.76f*i);
	}

	for( size_t i = 0; i < PACKAGE_ELEMENT_COUNT; ++i ){
		src[i] = src_data + i;
		dst[i] = dst_data + i;
		dest_ref[i].yzx( src_data[i].xyz() + src_data[i].wxy() );
	}

	fn( src, (void*)NULL, dst, (void*)NULL );

	for( size_t i = 0; i < PACKAGE_ELEMENT_COUNT; ++i ){
		BOOST_CHECK_CLOSE( dest_ref[i][0], dst_data[i][0], 0.00001f );
		BOOST_CHECK_CLOSE( dest_ref[i][1], dst_data[i][1], 0.00001f );
		BOOST_CHECK_CLOSE( dest_ref[i][2], dst_data[i][2], 0.00001f );
	}

	_aligned_free( src_data );
	_aligned_free( dst_data );
}
#endif

#if ALL_TESTS_ENABLED

__m128 to_mm( vec4 const& v ){
	__m128 tmp;
	*(vec4*)(&tmp) = v;
	return tmp;
}

__m128 to_mm( vec3 const& v ){
	__m128 tmp;
	*(vec4*)(&tmp) = vec4(v, 0.0f);
	return tmp;
}

vec4 to_vec4( __m128 const& v ){
	return *reinterpret_cast<vec4 const*>(&v);
}

struct intrinsic_ps_in
{
	vec3 in0;
	vec3 in1;
};

struct intrinsic_ps_out
{
	vec3 out0;
	vec2 out1;
};

BOOST_FIXTURE_TEST_CASE( ps_intrinsics, jit_fixture )
{
	init_ps( "./repo/question/v1a1/intrinsics.sps" );

	jit_function<void(void*, void*, void*, void*)> fn;
	function( fn, "fn" );

	intrinsic_ps_in*  in [PACKAGE_ELEMENT_COUNT] = {NULL};
	intrinsic_ps_out* out[PACKAGE_ELEMENT_COUNT] = {NULL};

	intrinsic_ps_in  in_data [PACKAGE_ELEMENT_COUNT];
	intrinsic_ps_out out_data[PACKAGE_ELEMENT_COUNT];

	for( size_t i = 0; i < PACKAGE_ELEMENT_COUNT; ++i ){
		for(size_t j = 0; j < 3; ++j)
		{
			size_t base = i * 3 + j;
			in_data[i].in0[j] = (base+3.77f)*(0.76f*base);
			in_data[i].in1[j] = (base-4.62f)*(0.11f*base);
		}
		in[i]  = in_data  + i;
		out[i] = out_data + i;
	}
	
	intrinsic_ps_out dest_ref[PACKAGE_ELEMENT_COUNT];
	for( int i = 0; i < PACKAGE_ELEMENT_COUNT; ++i )
	{
		// SSE dot
		vec4 prod = to_vec4( _mm_mul_ps( to_mm(in_data[i].in0), to_mm(in_data[i].in1) ) );
		float x = prod[0] + prod[1] + prod[2];

		// SSE prod
		__m128 a0 = to_mm(in_data[i].in0.yzx());
		__m128 a1 = to_mm(in_data[i].in1.zxy());
		__m128 b0 = to_mm(in_data[i].in0.zxy());
		__m128 b1 = to_mm(in_data[i].in1.yzx());
		__m128 first_prod = _mm_mul_ps(a0, a1);
		__m128 second_prod = _mm_mul_ps(b0, b1);
		__m128 f3_m = _mm_sub_ps( first_prod, second_prod );
		vec3 f3 = to_vec4( f3_m ).xyz();
		
		dest_ref[i].out0 = to_vec4( _mm_sqrt_ps(to_mm(in_data[i].in0)) ).xyz();
		dest_ref[i].out1[0] = x;
		dest_ref[i].out1[1] = sqrt(in_data[i].in0[0]);
	}

	fn( (void*)in, (void*)NULL, (void*)out, (void*)NULL );

	for( size_t i = 0; i < PACKAGE_ELEMENT_COUNT; ++i ){
		BOOST_CHECK_CLOSE( out_data[i].out0[0], dest_ref[i].out0[0], 0.00001f );
		BOOST_CHECK_CLOSE( out_data[i].out0[1], dest_ref[i].out0[1], 0.00001f );
		BOOST_CHECK_CLOSE( out_data[i].out0[2], dest_ref[i].out0[2], 0.00001f );

		BOOST_CHECK_CLOSE( out_data[i].out1[0], dest_ref[i].out1[0], 0.00001f );
		BOOST_CHECK_CLOSE( out_data[i].out1[1], dest_ref[i].out1[1], 0.00001f );
	}
}
#endif

#if ALL_TESTS_ENABLED
BOOST_FIXTURE_TEST_CASE( ps_branches, jit_fixture ){
	init_ps( "./repo/question/v1a1/branches.sps" );
	
	jit_function<void(void*, void*, void*, void*)> fn;
	function( fn, "fn" );

	BOOST_REQUIRE( fn );

	struct ps_in
	{
		float in0;
		vec3  in1;
	};

	struct ps_out
	{
		vec2 out;
	};

	ps_in   in_data [PACKAGE_ELEMENT_COUNT];
	ps_out  out_data[PACKAGE_ELEMENT_COUNT];
	ps_in*  in [PACKAGE_ELEMENT_COUNT] = {NULL};
	ps_out* out[PACKAGE_ELEMENT_COUNT] = {NULL};

	vec2 ref_out[PACKAGE_ELEMENT_COUNT];

	srand(0);
	for( int i = 0; i < PACKAGE_ELEMENT_COUNT; ++i){
		in[i]  = in_data  + i;
		out[i] = out_data + i;

		// Init Data
		in_data[i].in0 = (i * 0.34f) - 1.0f;
		for( int j = 0; j < 3; ++j ){
			in_data[i].in1[j] = rand() / 35.0f;
		}

		// Compute reference data
		ref_out[i][0] = 88.3f;
		ref_out[i][1] = 75.4f;

		
		if( in_data[i].in0 > 0.0f ){
			ref_out[i][0] = in_data[i].in0;
		}

		if( in_data[i].in0 > 1.0f ){
			ref_out[i][1] = in_data[i].in1[0];
		} else {
			ref_out[i][1] = in_data[i].in1[1];
		}

		if( in_data[i].in0 > 2.0f ){
			ref_out[i][1] = in_data[i].in1[2];
			if ( in_data[i].in0 > 3.0f ){
				ref_out[i][1] = ref_out[i][1] + 1.0f;
			} else if( in_data[i].in0 > 2.5f ){
				ref_out[i][1] = ref_out[i][1] + 2.0f;
			}
		}
	}

	fn( (void*)in, (void*)NULL, (void*)out, (void*)NULL );

	for( size_t i = 0; i < PACKAGE_ELEMENT_COUNT; ++i ){
		BOOST_CHECK_CLOSE( out_data[i].out[0], ref_out[i][0], 0.00001f );
		BOOST_CHECK_CLOSE( out_data[i].out[1], ref_out[i][1], 0.00001f );
	}
}
#endif

#if ALL_TESTS_ENABLED

template<typename T, typename MemberPtr>
void get_ddx(T* out, T const* in, MemberPtr ptr)
{
	int const LINES = PACKAGE_ELEMENT_COUNT / PACKAGE_LINE_ELEMENT_COUNT;

	for( int col = 0; col < PACKAGE_LINE_ELEMENT_COUNT; col+=2 ){
		for( int row = 0; row < LINES; ++row ){
			int index = row * PACKAGE_LINE_ELEMENT_COUNT + col; 
			out[index+1].*ptr = out[index].*ptr =
				in[index+1].*ptr - in[index].*ptr;
		}
	}
}

template<typename T, typename MemberPtr>
void get_ddy(T* out, T const* in, MemberPtr ptr)
{
	int const LINES = PACKAGE_ELEMENT_COUNT / PACKAGE_LINE_ELEMENT_COUNT;

	for( int row = 0; row < LINES; row+=2 ){
		for( int col = 0; col < PACKAGE_LINE_ELEMENT_COUNT; ++col ){
			int index = row * PACKAGE_LINE_ELEMENT_COUNT + col;
			int next_row_index = index + PACKAGE_LINE_ELEMENT_COUNT;
			out[next_row_index].*ptr = out[index].*ptr =
				in[next_row_index].*ptr - in[index].*ptr;
		}
	}
}

BOOST_FIXTURE_TEST_CASE( ddx_ddy, jit_fixture ){
	init_ps( "./repo/question/v1a1/ddx_ddy.sps" );

	jit_function<void(void*, void*, void*, void*)> fn;
	function( fn, "fn" );

	BOOST_REQUIRE( fn );

	struct ps_in_out
	{
		float v0;
		vec2  v1;
		vec3  v2;
		vec4  v3;
	};

	ps_in_out  in_data [PACKAGE_ELEMENT_COUNT];
	ps_in_out  out_data[PACKAGE_ELEMENT_COUNT];
	ps_in_out* in [PACKAGE_ELEMENT_COUNT];
	ps_in_out* out[PACKAGE_ELEMENT_COUNT];

	ps_in_out  ddx_out[PACKAGE_ELEMENT_COUNT];
	ps_in_out  ddy_out[PACKAGE_ELEMENT_COUNT];

	ps_in_out  ref_out[PACKAGE_ELEMENT_COUNT];

	srand(0);

	// Init Data
	for( int i = 0; i < PACKAGE_ELEMENT_COUNT; ++i){
		in[i]  = in_data + i;
		out[i] = out_data + i;

		in_data[i].v0    = rand() / 67.0f;
		in_data[i].v1[0] = rand() / 67.0f;
		in_data[i].v1[1] = rand() / 67.0f;
		in_data[i].v2[0] = rand() / 67.0f;
		in_data[i].v2[1] = rand() / 67.0f;
		in_data[i].v2[2] = rand() / 67.0f;
		in_data[i].v3[0] = rand() / 67.0f;
		in_data[i].v3[1] = rand() / 67.0f;
		in_data[i].v3[2] = rand() / 67.0f;
		in_data[i].v3[3] = rand() / 67.0f;
	}

	get_ddx(ddx_out, in_data, &ps_in_out::v0);
	get_ddx(ddx_out, in_data, &ps_in_out::v1);
	get_ddx(ddx_out, in_data, &ps_in_out::v2);
	get_ddx(ddx_out, in_data, &ps_in_out::v3);

	get_ddy(ddy_out, in_data, &ps_in_out::v0 );
	get_ddy(ddy_out, in_data, &ps_in_out::v1 );
	get_ddy(ddy_out, in_data, &ps_in_out::v2 );
	get_ddy(ddy_out, in_data, &ps_in_out::v3 );

	for( int i = 0; i < PACKAGE_ELEMENT_COUNT; ++i ){
		ref_out[i].v0 = ddx_out[i].v0			+ ddy_out[i].v0;
		ref_out[i].v1 = ddx_out[i].v1.xy()		+ ddy_out[i].v1.yx();
		ref_out[i].v2 = ddx_out[i].v2.xyz()		+ ddy_out[i].v2.yzx();
		ref_out[i].v3 = ddx_out[i].v3.xwzy()	+ ddy_out[i].v3.yzxw();
	}

	fn( (void*)in, (void*)NULL, (void*)out, (void*)NULL );

	for( size_t i = 0; i < PACKAGE_ELEMENT_COUNT; ++i ){
		BOOST_CHECK_CLOSE( out_data[i].v0,    ref_out[i].v0,    0.00001f );
		BOOST_CHECK_CLOSE( out_data[i].v1[0], ref_out[i].v1[0], 0.00001f );
		BOOST_CHECK_CLOSE( out_data[i].v1[1], ref_out[i].v1[1], 0.00001f );
		BOOST_CHECK_CLOSE( out_data[i].v2[0], ref_out[i].v2[0], 0.00001f );
		BOOST_CHECK_CLOSE( out_data[i].v2[1], ref_out[i].v2[1], 0.00001f );
		BOOST_CHECK_CLOSE( out_data[i].v2[2], ref_out[i].v2[2], 0.00001f );
		BOOST_CHECK_CLOSE( out_data[i].v3[0], ref_out[i].v3[0], 0.00001f );
		BOOST_CHECK_CLOSE( out_data[i].v3[1], ref_out[i].v3[1], 0.00001f );
		BOOST_CHECK_CLOSE( out_data[i].v3[2], ref_out[i].v3[2], 0.00001f );
		BOOST_CHECK_CLOSE( out_data[i].v3[3], ref_out[i].v3[3], 0.00001f );
	}
}

#endif

#if ALL_TESTS_ENABLED

struct sampler_t{
	uintptr_t ss, tex;
};

void tex2Dlod_ps(vec4* ret, uint32_t /*mask*/, sampler_t* s, vec4* t)
{
	BOOST_CHECK_EQUAL( s->ss, 0xF3DE89C );
	BOOST_CHECK_EQUAL( s->tex, 0xB785D3A );
	*ret = t->zyxw() + t->wxzy();
}

BOOST_FIXTURE_TEST_CASE( tex_ps, jit_fixture )
{
	init_ps( "./repo/question/v1a1/tex.sps" );

	set_raw_function( &tex2Dlod_ps, "sasl.ps.tex2d.lod" );

	jit_function<void(void*, void*, void*, void*)> fn;
	function( fn, "fn" );

	BOOST_REQUIRE( fn );

	vec4* in [PACKAGE_ELEMENT_COUNT] = {NULL};
	vec4* out[PACKAGE_ELEMENT_COUNT] = {NULL};

	vec4  in_data [PACKAGE_ELEMENT_COUNT];
	vec4  out_data[PACKAGE_ELEMENT_COUNT];

	vec4  out_ref[PACKAGE_ELEMENT_COUNT];

	srand(0);
	for( size_t i = 0; i < PACKAGE_ELEMENT_COUNT * 4; ++i ){
		((float*)in_data)[i] = rand() / 177.8f;
	}

	for( size_t i = 0; i < PACKAGE_ELEMENT_COUNT; ++i ){
		in[i]  = in_data + i;
		out[i] = out_data + i;
		out_ref[i] = in_data[i].zyxw() + in_data[i].wxzy();
	}
	sampler_t smpr;
	
	smpr.ss = 0xF3DE89C;
	smpr.tex = 0xB785D3A;

	sampler_t* psmpr = &smpr;
	fn(in, (void*)&psmpr, out, (void*)NULL);

	for( size_t i = 0; i < PACKAGE_ELEMENT_COUNT; ++i ){
		BOOST_CHECK_CLOSE( out_ref[i][0], out_data[i][0], 0.00001f );
		BOOST_CHECK_CLOSE( out_ref[i][1], out_data[i][1], 0.00001f );
		BOOST_CHECK_CLOSE( out_ref[i][2], out_data[i][2], 0.00001f );
	}
}

#endif

#if ALL_TESTS_ENABLED

BOOST_FIXTURE_TEST_CASE( ps_for_loop, jit_fixture ){
	init_ps( "./repo/question/v1a1/for_loop.sps" );

	jit_function<void(void*, void*, void*, void*)> fn;
	function( fn, "fn" );

	BOOST_REQUIRE( fn );

	float in_data [PACKAGE_ELEMENT_COUNT];
	float out_data[PACKAGE_ELEMENT_COUNT];

	float* in [PACKAGE_ELEMENT_COUNT] = {NULL};
	float* out[PACKAGE_ELEMENT_COUNT] = {NULL};
	float  ref_out[PACKAGE_ELEMENT_COUNT];

	srand(0);
	for( int i = 0; i < PACKAGE_ELEMENT_COUNT; ++i){
		// Init Data
		in_data[i] = rand() / 1000.0f;
		in[i] = in_data + i;
		out[i] = out_data + i;

		float x = in_data[i];
		for( int j = 0; j < 10; ++j )
		{
			x *= 2.0f;
			if ( x > 5000.0f ){ break; }
		}

		ref_out[i] = x;
	}

	fn( (void*)in, (void*)NULL, (void*)out, (void*)NULL );

	for( size_t i = 0; i < PACKAGE_ELEMENT_COUNT; ++i ){
		BOOST_CHECK_CLOSE( out_data[i], ref_out[i], 0.00001f );
	}
}

#endif

#if ALL_TESTS_ENABLED

BOOST_FIXTURE_TEST_CASE( ps_while, jit_fixture ){
	init_ps( "./repo/question/v1a1/while.sps" );

	jit_function<void(void*, void*, void*, void*)> fn;
	function( fn, "fn" );

	BOOST_REQUIRE( fn );

	float in_data [PACKAGE_ELEMENT_COUNT];
	float out_data[PACKAGE_ELEMENT_COUNT];

	float* in [PACKAGE_ELEMENT_COUNT] = {NULL};
	float* out[PACKAGE_ELEMENT_COUNT] = {NULL};
	float  ref_out[PACKAGE_ELEMENT_COUNT];

	srand(0);
	for( int i = 0; i < PACKAGE_ELEMENT_COUNT; ++i){
		// Init Data
		in_data[i] = 0.79f * ( i * i * i );
		in[i] = in_data + i;
		out[i] = out_data + i;

		float x = in_data[i];
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
		BOOST_CHECK_CLOSE( out_data[i], ref_out[i], 0.00001f );
		BOOST_CHECK_CLOSE( out_data[i], ref_out[i], 0.00001f );
	}
}

#endif

#if ALL_TESTS_ENABLED

BOOST_FIXTURE_TEST_CASE( ps_do_while, jit_fixture ){
	init_ps( "./repo/question/v1a1/do_while.sps" );

	jit_function<void(void*, void*, void*, void*)> fn;
	function( fn, "fn" );

	BOOST_REQUIRE( fn );

	float in_data [PACKAGE_ELEMENT_COUNT];
	float out_data[PACKAGE_ELEMENT_COUNT];

	float* in [PACKAGE_ELEMENT_COUNT] = {NULL};
	float* out[PACKAGE_ELEMENT_COUNT] = {NULL};
	float  ref_out[PACKAGE_ELEMENT_COUNT];

	srand(0);
	for( int i = 0; i < PACKAGE_ELEMENT_COUNT; ++i){
		// Init Data
		in_data[i] = 0.79f * ( i * i * i );
		in[i] = in_data + i;
		out[i] = out_data + i;

		float x = in_data[i];
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
		BOOST_CHECK_CLOSE( out_data[i], ref_out[i], 0.00001f );
		BOOST_CHECK_CLOSE( out_data[i], ref_out[i], 0.00001f );
	}
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

	BOOST_CHECK_EQUAL( ref_v0[0], v0[0] );
	BOOST_CHECK_EQUAL( ref_v0[1], v0[1] );

	BOOST_CHECK_EQUAL( ref_v1[0], v1[0] );
	BOOST_CHECK_EQUAL( ref_v1[1], v1[1] );
	BOOST_CHECK_EQUAL( ref_v1[2], v1[2] );

	BOOST_CHECK_CLOSE( ref_v2[0], v2[0], 0.000001f );
	BOOST_CHECK_CLOSE( ref_v2[1], v2[1], 0.000001f );
	BOOST_CHECK_CLOSE( ref_v2[2], v2[2], 0.000001f );

	BOOST_CHECK_CLOSE( ref_v3[0], v3[0], 0.000001f );
	BOOST_CHECK_CLOSE( ref_v3[1], v3[1], 0.000001f );
	BOOST_CHECK_CLOSE( ref_v3[2], v3[2], 0.000001f );
	BOOST_CHECK_CLOSE( ref_v3[3], v3[3], 0.000001f );

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

#if ALL_TESTS_ENABLED
BOOST_FIXTURE_TEST_CASE( arith_ops, jit_fixture )
{
	init_g( "./repo/question/v1a1/arithmetic.ss" );

	JIT_FUNCTION( vec4(vec4),					test_float_arith );
	JIT_FUNCTION( int3(int3),					test_int_arith );
	JIT_FUNCTION( float3x4(float3x4,float3x4),	test_mat_arith );
	JIT_FUNCTION( int3(int3,int),				test_vec_scalar_arith );
	JIT_FUNCTION( float3x4(float3x4, float),	test_mat_scalar_arith );

	vec4 vf( 76.8f, -88.5f, 37.7f, -98.1f );
	int3 vi( 87, 46, 22 );
	int3 zi( 0, 0, 0 );

	vec4 ref_f( vf[0]+vf[1], vf[1]-vf[2], vf[2]*vf[3], vf[3]/vf[0] );
	int4 ref_i( vi[0]/vi[1], vi[1]%vi[2], vi[2]*vi[0] );

	vec4 ret_f = test_float_arith(vf);
	int3 ret_i = test_int_arith(vi);
	int3 ret_zi = test_int_arith(zi);	// A special test for div and mod by zero.

	BOOST_CHECK_CLOSE( ref_f[0], ret_f[0], 0.000001f );
	BOOST_CHECK_CLOSE( ref_f[1], ret_f[1], 0.000001f );
	BOOST_CHECK_CLOSE( ref_f[2], ret_f[2], 0.000001f );
	BOOST_CHECK_CLOSE( ref_f[3], ret_f[3], 0.000001f );

	BOOST_CHECK_EQUAL( ref_i[0], ret_i[0] );
	BOOST_CHECK_EQUAL( ref_i[1], ret_i[1] );
	BOOST_CHECK_EQUAL( ref_i[2], ret_i[2] );

	{
		float lhs_array[3][4] =
		{
			{17.7f, 66.3f, 0.92f, -88.7f},
			{8.6f, -0.22f, 17.1f, -64.4f},
			{199.8f, 0.1f, -0.1f, 99.73f}
		};

		float rhs_array[3][4] =
		{
			{9.62f, 10.33f, -18.2f, 99.7f},
			{-0.3f, -76.9f, 93.3f,  0.22f},
			{44.1f, 0.027f, 19.9f, -33.5f}
		};

		float ref_v[3][4] = {0};

		for( int i = 0; i < 3; ++i )
		{
			for( int j = 0; j < 4; ++j )
			{
				ref_v[i][j] 
				= (lhs_array[i][j] + rhs_array[i][j]) * lhs_array[i][j]
				- fmodf( (rhs_array[i][j] / lhs_array[i][j]), rhs_array[i][j] );
			}
		}

		float3x4& lhs( reinterpret_cast<float3x4&>(lhs_array) );
		float3x4& rhs( reinterpret_cast<float3x4&>(rhs_array) );
		float3x4 ret = test_mat_arith( lhs, rhs );

		for( int i = 0; i < 3; ++i )
		{
			for( int j = 0; j < 4; ++j )
			{
				BOOST_CHECK_CLOSE( ref_v[i][j], ret.data_[i][j], 0.000012f );
			}
		}
	}

	{
		int arr[3] = {7, 0, -3};
		int3& x = ( reinterpret_cast<int3&>(arr) );
		int y = 876;

		int3 ret = test_vec_scalar_arith(x, y);

		for (int i = 0; i < 3; ++i){
			BOOST_CHECK_EQUAL( 0-y/((arr[i]*6)?(arr[i]*6):1)+3, ret.data_[i] );
		}
	}

	{
		float arr[3][4] =
		{
			{17.7f, 66.3f, 0.92f, -88.7f},
			{8.6f, -0.22f, 17.1f, -64.4f},
			{199.8f, 0.1f, -0.0f, 99.73f}
		};

		float3x4&	x( reinterpret_cast<float3x4&>(arr) );
		float		y = -0.33f;

		float3x4 ret = test_mat_scalar_arith(x, y);
		for( int i = 0; i < 3; ++i ) {
			for( int j = 0; j < 4; ++j ) {
				float ref_v = 7.0f-y/(arr[i][j]*0.5f)+3.3f;
				if( *(int*)(&ref_v) != *(int*)(&ret.data_[i][j]) ){
					BOOST_CHECK_CLOSE( ret.data_[i][j], ref_v, 0.000001f );
				}
			}
		}
	}
}
#endif

#if ALL_TESTS_ENABLED

struct uint4
{
	uint32_t v[4];
};

BOOST_FIXTURE_TEST_CASE( bit_ops, jit_fixture )
{
	init_g( "./repo/question/v1a1/bit_ops.ss" );

	jit_function<uint32_t(uint4, uint32_t)> test_bitwise_ops;
	function( test_bitwise_ops, "test_bitwise_ops" );
	
	uint4	 a = { {0x3C657DBAU, 13, 0x76337BEC, 4} };
	uint32_t b(0xCB6F34A3);
	uint64_t ref_v = ( (a.v[0]<<a.v[1]) + (a.v[1]<<3u) - (a.v[1]>>2u) ) & (a.v[2]>>a.v[3]) | b;
	uint64_t ret_v = test_bitwise_ops(a, b);

	BOOST_CHECK_EQUAL( ref_v, ret_v );
}
#endif

#if ALL_TESTS_ENABLED

int do_arith_assign( int v0, int v1 )
{
	int v = (v0%=(v1?v1:1));
	int vv= (v1*=v0);
	return (v0+=v1)-=(vv/=(v?v:1));
}

int do_bit_assign(int v0, int v1)
{
	int v = (v0&=v1);
	int vv= (v1|=v0);
	return v0^=(vv-v);
}

BOOST_FIXTURE_TEST_CASE( assigns, jit_fixture )
{
	init_g( "./repo/question/v1a1/assigns.ss" );

	JIT_FUNCTION( int2x3(int2x3,int2x3), test_arith_assign );
	JIT_FUNCTION( int2x3(int2x3,int2x3), test_bit_assign );
	JIT_FUNCTION( int(int,int),			 test_scalar_arith_assign );
	
	int32_t lhs_arr[2][3] =
	{
		{ 786, 0, 33769097 },
		{ -1, -3899927, 67}
	};

	int32_t rhs_arr[2][3] =
	{
		{ 0, 87927877, -9728 },
		{ 788, -3899927, 67}
	};

	int2x3 lhs( reinterpret_cast<int2x3&>(lhs_arr) );
	int2x3 rhs( reinterpret_cast<int2x3&>(rhs_arr) );

	int scalar_ret = test_scalar_arith_assign(-1, 788);
	BOOST_CHECK_EQUAL( scalar_ret, do_arith_assign(-1, 788) );

	int2x3 arith_ret = test_arith_assign(lhs, rhs);
	int2x3 bit_ret = test_bit_assign(lhs, rhs);
	for( int i = 0; i < 2; ++i )
	{
		for( int j = 0; j < 3; ++j )
		{
			BOOST_CHECK_EQUAL( arith_ret.data_[i][j], do_arith_assign(lhs_arr[i][j], rhs_arr[i][j]) );
			BOOST_CHECK_EQUAL( bit_ret.data_[i][j], do_bit_assign(lhs_arr[i][j], rhs_arr[i][j]) );
		}
	}

}
#endif

#if ALL_TESTS_ENABLED
BOOST_FIXTURE_TEST_CASE( array_and_index, jit_fixture )
{
	init_g( "./repo/question/v1a1/array_and_index.ss" );

	JIT_FUNCTION( vec4(float3x4),  test_mat_index );
	JIT_FUNCTION( float(float3x4), test_vec_index );

	{
		float arr[3][4] = { {0.0f,1.0f,2.0f,3.0f}, {4.0f,5.0f,6.0f,7.0f}, {8.0f,9.0f,10.0f,11.0f} };
		float3x4& m34 = reinterpret_cast<float3x4&>(arr);
		vec4 ret = test_mat_index(m34);
		float ref_v[4] = {4.0f,6.0f,8.0f,10.0f};
		for( int i = 0; i < 4; ++i )
		{
			BOOST_CHECK_CLOSE( ret.data_[i], ref_v[i], 0.000001f );
		}
	}

	{
		float arr[3][4] = { {0.0f,1.0f,2.0f,3.0f}, {4.0f,5.0f,6.0f,7.0f}, {8.0f,9.0f,10.0f,11.0f} };
		float3x4& m34 = reinterpret_cast<float3x4&>(arr);
		float ret = test_vec_index(m34);
		for( int i = 0; i < 4; ++i )
		{
			BOOST_CHECK_CLOSE( ret, 15.0f, 0.000001f );
		}
	}
}
#endif

#if ALL_TESTS_ENABLED

struct array_vertex_data
{
	vec4 pos;
	int4 ids;
};

struct array_vs_sin
{
	vec4* pos;
	int4* ids;
};

struct array_vs_bin
{
	int		mat_size;
	mat44*	mat_arr;
};

struct array_vs_bout
{
	vec4 pos;
};

vec4 my_transform(mat44& m, vec4& v)
{
	vec4 ret(0.0f, 0.0f, 0.0f, 0.0f);

#if defined(EFLIB_CPU_X86) || defined(EFLIB_CPU_X64)
	__m128 col;
	__m128 v4f;
	for(int i = 0; i < 4; ++i)
	{
		for(int j = 0; j < 4; ++j)
		{
			col.m128_f32[j] = m.data_[i][j];
			v4f.m128_f32[j] = v.data_[j];
		}
		__m128 elem_v4f = _mm_mul_ps(v4f, col);

		for(int j = 0; j < 4; ++j)
		{
			ret[i] += elem_v4f.m128_f32[j];
		}
	}
#else
	eflib::transform(ret, m, v);
#endif

	return ret;
}

BOOST_FIXTURE_TEST_CASE( array_test, jit_fixture )
{
	init_vs( "./repo/question/v1a1/array.svs" );
	JIT_FUNCTION( void(array_vs_sin*, array_vs_bin*, void*, array_vs_bout*), fn );

	srand(887);

	mat44 mats[50];
	for(int i = 0; i < 50; ++i)
	{
		for( mat44::iterator it = mats[i].begin(); it != mats[i].end(); ++it )
		{
			*it = rand() / 227.0f;
		}
	}

	array_vertex_data data;
	data.pos = vec4(-10.0f, 77.8f, 8.3f, -87.3f);
	
	array_vs_sin sin;
	sin.ids = &data.ids;
	sin.pos = &data.pos;

	array_vs_bin bin;
	bin.mat_size = 50;
	bin.mat_arr = &mats[0];
	
	array_vs_bout bout;

	for(int i = 0; i < 100; ++i)
	{
		data.ids = int4(rand()%50, rand()%50, rand()%50, rand()%50);
		fn(&sin, &bin, (void*)NULL, &bout);

		vec4 ref_v = vec4(0.0f, 0.0f, 0.0f, 0.0f);
		for(int i = 0; i < 4; ++i)
		{
			ref_v += my_transform(mats[data.ids[i]], data.pos);
		}
		ref_v /= 4.0f;

		BOOST_CHECK_CLOSE( ref_v[0], bout.pos[0], 0.000001f );
		BOOST_CHECK_CLOSE( ref_v[1], bout.pos[1], 0.000001f );
		BOOST_CHECK_CLOSE( ref_v[2], bout.pos[2], 0.000001f );
		BOOST_CHECK_CLOSE( ref_v[3], bout.pos[3], 0.000001f );
	}
}
#endif

#if ALL_TESTS_ENABLED
BOOST_FIXTURE_TEST_CASE(input_assigned, jit_fixture)
{
	init_vs("./repo/question/v1a1/input_assigned.svs");

	JIT_FUNCTION( void(vec4**, float*, void*, vec4*), fn );

	vec4  pos(0.3f, -0.6f, 2.2f, 8.0f);
	vec4  out;
	vec4* sin = &pos;
	float x = 9.7f;

	float new_x = x;
	new_x += 0.5f;
	vec4 new_pos = pos;
	new_pos[0] += new_x;
	vec4 new_out = new_pos;
	new_out[0] += 0.5f;

	fn(&sin, &x, (void*)NULL, &out);

	BOOST_CHECK_EQUAL(pos[0], 0.3f );
	BOOST_CHECK_EQUAL(pos[1], -0.6f);
	BOOST_CHECK_EQUAL(pos[2], 2.2f );
	BOOST_CHECK_EQUAL(pos[3], 8.0f );

	BOOST_CHECK_EQUAL(x, 9.7f);

	BOOST_CHECK_EQUAL(out[0], new_out[0]);
	BOOST_CHECK_EQUAL(out[1], new_out[1]);
	BOOST_CHECK_EQUAL(out[2], new_out[2]);
	BOOST_CHECK_EQUAL(out[3], new_out[3]);
}
#endif
BOOST_AUTO_TEST_SUITE_END();
