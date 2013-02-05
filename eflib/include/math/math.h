#pragma once

#ifndef EFLIB_MATH_MATH_H
#define EFLIB_MATH_MATH_H

#include <eflib/include/platform/typedefs.h>
#include <eflib/include/platform/constant.h>
#include <eflib/include/diagnostics/assert.h>
#include <eflib/include/math/vector.h>
#include <eflib/include/math/matrix.h>

#ifndef EFLIB_NO_SIMD
#include <emmintrin.h>
#endif

#include <eflib/include/platform/boost_begin.h>
#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>
#include <eflib/include/platform/boost_end.h>

#include <limits>
#include <cassert>

namespace eflib{
	template <class T>
	T sign(const T& in)
	{
		return in >= T(0) ? T(1) : T(-1);
	}

	template <class T>
	bool equal(const T& in1, const T& in2)
	{
		if((T)(std::abs(in1 - in2)) <= std::numeric_limits<T>::epsilon()) return true;
		return false;
	}

	inline bool equal(const vec2& v1, const vec2& v2){
		return 
			equal(v1.x(), v2.x()) &&
			equal(v1.y(), v2.y());
	}

	inline bool equal(const vec3& v1, const vec3& v2)
	{
		return 
			equal(v1.x(), v2.x()) &&
			equal(v1.y(), v2.y()) &&
			equal(v1.z(), v2.z());
	}

	inline bool equal(const vec4& v1, const vec4& v2)
	{
		return 
			equal(v1.x(), v2.x()) &&
			equal(v1.y(), v2.y()) &&
			equal(v1.z(), v2.z()) &&
			equal(v1.w(), v2.w())
			;
	}

	template <class T, class U>
	T round(U d)
	{
		BOOST_STATIC_ASSERT(boost::is_floating_point<U>::value);
		BOOST_STATIC_ASSERT(boost::is_integral<T>::value);
		return (T)(d+0.5);
	}

	template <class T>
	T trunc(T d)
	{
		BOOST_STATIC_ASSERT(boost::is_floating_point<T>::value);
		return d > static_cast<T>(0) ? fast_floor(d) : fast_ceil(d);
	}

	template <class T, class U>
	void round(T& t, U d)
	{
		BOOST_STATIC_ASSERT(boost::is_floating_point<U>::value);
		BOOST_STATIC_ASSERT(boost::is_integral<T>::value);
		t = (T)floor(d+0.5);
	}

	template <class T>
	T clamp(T v, T minv, T maxv)
	{
		BOOST_STATIC_ASSERT(boost::is_arithmetic<T>::value);
		assert(minv <= maxv);

		if(v < minv) return minv;
		if(v > maxv) return maxv;
		return v;
	}

	template <class T>
	T radians(T angle)
	{
		BOOST_STATIC_ASSERT(boost::is_float<T>::value);
		
		return T(angle * PI / T(180.0));
	}

	template <class T>
	void sincos(T rad, float& s, float& c)
	{
		BOOST_STATIC_ASSERT(boost::is_float<T>::value);
		s = sin(rad);
		c = cos(rad);
	}

	inline float log2(float f){return std::log(f) * 1.442695f;}

	// From http://www.musicdsp.org/showone.php?id=63 & http://www.flipcode.com/archives/Fast_log_Function.shtml
	inline float fast_log2(float val)
	{
		union INTORFLOAT
		{
			int i;
			float f;
		};
		
		INTORFLOAT iof;
		iof.f = val;
		int x = iof.i;
		int log_2 = ((x >> 23) & 255) - 128;
		x &= ~(255 << 23);
		x += 127 << 23;
		iof.i = x;
		iof.f = ((-1.0f / 3) * iof.f + 2) * iof.f - 2.0f / 3;

		return iof.f + log_2;
	}

	inline float fast_log(float val)
	{
		return fast_log2(val) * 0.69314718f;
	}

	inline float fast_round(float val)
	{
		union INTORFLOAT
		{
			int i;
			float f;
		};

		INTORFLOAT n;
		INTORFLOAT bias;
		n.f = val;
		bias.i = ((23 + 127) << 23) + (n.i & 0x80000000);
		n.f += bias.f;
		n.f -= bias.f;
		return n.f;
	}

	inline float fast_ceil(float val)
	{
		float f = fast_round(val);
		return (f < val) ? f + 1 : f;
	}

	inline float fast_floor(float val)
	{
		float f = fast_round(val);
		return (f > val) ? f - 1 : f;
	}

	// From http://www.stereopsis.com/sree/fpu2006.html
	inline int fast_roundi(double d)
	{
		return static_cast<int>( floor(d+0.5) );
		/*
		const double DOUBLE_MAGIC = 6755399441055744.0; // 2^51 + 2^52
		
		union INTORDOUBLE
		{
			int i;
			double d;
		};

		INTORDOUBLE n;
		n.d = d + DOUBLE_MAGIC;
		return n.i;
		*/
	}

	inline int fast_ceili(double d)
	{
		const double DOUBLE_MAGIC_ROUND_EPS = (0.5f - 1.5e-8);	//almost .5f = .5f - 1e^(number of exp bit)
		return fast_roundi(d + DOUBLE_MAGIC_ROUND_EPS);
	}

	inline int fast_floori(double d)
	{
		const double DOUBLE_MAGIC_ROUND_EPS = (0.5f - 1.5e-8);	//almost .5f = .5f - 1e^(number of exp bit)
		return fast_roundi(d - DOUBLE_MAGIC_ROUND_EPS);
	}

#ifndef EFLIB_NO_SIMD
	inline int fast_ftol(float f)
	{
		return _mm_cvtt_ss2si(_mm_load_ss(&f));
	}
#else
	inline int fast_ftol(double d)
	{
		const double DOUBLE_MAGIC_ROUND_EPS = (0.5f - 1.5e-8);	//almost .5f = .5f - 1e^(number of exp bit)
		return fast_roundi(d < 0 ? d + DOUBLE_MAGIC_ROUND_EPS : d - DOUBLE_MAGIC_ROUND_EPS);
	}
#endif

	inline int ceil_to_pow2( int i )
	{
		--i;
		i |= i >> 1;
		i |= i >> 2;
		i |= i >> 4;
		i |= i >> 8;
		i |= i >> 16;
		++i;

		return i;
	}

	inline uint32_t count_bits(uint32_t v)
	{
		v = v - ( (v >> 1) & 0x55555555 );                    // reuse input as temporary
		v = (v & 0x33333333) + ( (v >> 2) & 0x33333333 );     // temp
		return ( (v + (v >> 4) & 0xF0F0F0F) * 0x1010101 ) >> 24; // count
	}

	template <typename T> T count_bits(T v)
	{
		T c = 0;
		while(i)
		{
			++c;
			v &= v-1;
		}
		return c;
	}

	//////////////////////////////////////
	// base vector function
	//////////////////////////////////////
	vec2 normalize2(const vec2& v);
	vec3 normalize3(const vec3& v);
	vec4 normalize4(const vec4& v);

	//////////////////////////////////////
	//	blas level 1: vector - vector
	//////////////////////////////////////

	float dot_prod2(const vec2& v1, const vec2& v2);
	float dot_prod3(const vec3& v1, const vec3& v2);
	float dot_prod4(const vec4& v1, const vec4& v2);

	float cross_prod2(const vec2& v1, const vec2& v2);
	vec3 cross_prod3(const vec3& v1, const vec3& v2);

	vec2 clampps(const vec2& v, const vec2& minv, const vec2& maxv);
	vec3 clampps(const vec3& v, const vec3& minv, const vec3& maxv);
	vec4 clampps(const vec4& v, const vec4& minv, const vec4& maxv);

	vec2 clampss(const vec2& v, float min, float max);
	vec3 clampss(const vec3& v, float min, float max);
	vec4 clampss(const vec4& v, float min, float max);

	template <class Vec>
	Vec lerp(const Vec& v1, const Vec& v2, float t)
	{
		return Vec(v1 + (v2 - v1) * t);
	}

	vec3 reflect3(const vec3& i, const vec3& n);
	vec4 reflect4(const vec4& i, const vec4& n);

	vec3 refract3(const vec3& i, const vec3& n, float eta);
	vec4 refract4(const vec4& i, const vec4& n, float eta);

	float smoothstep(float min_v, float max_v, float v);

	//v1,v2,v3 is counter-clockwise.
	vec4& gen_plane(vec4& out, const vec4& v1, const vec4& v2, const vec4& v3);
	vec4& hermite(vec4& out, const vec4& v0, const vec4& v1, const vec4& v2, const vec4& v3);
	vec4& cutmull_rom(vec4& out, const vec4& v0, const vec4& v1, const vec4& v2, const vec4& v3);

	//////////////////////////////////////////////
	//  blas level 2: matrix - vector
	//////////////////////////////////////////////
	vec4& transform(vec4& out, const vec4& v, const mat44& m);
	vec4& transform(vec4& out, const mat44& m, const vec4& v);
	vec4& transform_coord(vec4& out, const vec4& v, const mat44& m);
	vec4& transform_normal(vec4& out, const vec4& v, const mat44& m);
	vec4& transform33(vec4& out, const vec4& v, const mat44& m);

	//////////////////////////////////////////////
	// blas level 3: matrix - matrix
	//////////////////////////////////////////////
	mat44& mat_mul(mat44& out, const mat44& m1, const mat44& m2);
	mat44& mat_transpose(mat44& out, const mat44& m1);
	mat44& mat_inverse(mat44& out, const mat44& m1);

	mat44& mat_identity(mat44& out);
	mat44& mat_zero(mat44& out);

	/////////////////////////////////////////////
	//  matrix generator : for mathematics
	/////////////////////////////////////////////
	mat44& mat_rotate(mat44& out, const vec4& axis, float delta);
	mat44& mat_rotX(mat44& out, float delta);
	mat44& mat_rotY(mat44& out, float delta);
	mat44& mat_rotZ(mat44& out, float delta);
	mat44& mat_translate(mat44& out, float x, float y, float z);
	mat44& mat_scale(mat44& out, float sx, float sy, float sz);
	mat44& mat_reflect(mat44& out, const vec4& plane);

	mat44& mat_lookat(mat44& out, const vec3& eye, const vec3& target, const vec3& up);

	mat44& mat_projection(mat44& out, float l, float r, float b, float t, float n, float f);
	mat44& mat_perspective(mat44& out, float w, float h, float n, float f);
	mat44& mat_perspective_fov(mat44& out, float fovy, float aspect, float n, float f);
	mat44& mat_ortho(mat44& out, float l, float r, float b, float t, float n, float f);

#if defined(EFLIB_CPU_X86) || defined(EFLIB_CPU_X64)
	inline __m128&			to_m128(vec4& v)
	{
		return reinterpret_cast<__m128&>(v);
	}

	inline __m128 const&	to_m128(vec4 const& v)
	{
		return reinterpret_cast<__m128 const&>(v);
	}

	inline vec4&			to_vec4(__m128& v)
	{
		return reinterpret_cast<vec4&>(v);
	}

	inline vec4 const&		to_vec4(__m128 const& v)
	{
		return reinterpret_cast<vec4 const&>(v);
	}

#if !defined(EFLIB_NO_SIMD)
	inline vec4& sse_add_assign(vec4& dst, vec4 const& src)
	{
		to_m128(dst) = _mm_add_ps( to_m128(dst), to_m128(src) );
		return dst;
	}
#endif

#endif
}
#endif