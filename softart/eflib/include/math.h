#ifndef EFLIB_MATH_H
#define EFLIB_MATH_H

#include "platform.h"
#include "detail/vector.h"
#include "detail/matrix.h"
#include "detail/quaternion.h"
#include "detail/collision_detection.h"

#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>

#include <limits>

namespace efl{
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
			equal(v1.x, v2.x) &&
			equal(v1.y, v2.y);
	}

	inline bool equal(const vec3& v1, const vec3& v2)
	{
		return 
			equal(v1.x, v2.x) &&
			equal(v1.y, v2.y) &&
			equal(v1.z, v2.z);
	}

	inline bool equal(const vec4& v1, const vec4& v2)
	{
		return 
			equal(v1.x, v2.x) &&
			equal(v1.y, v2.y) &&
			equal(v1.z, v2.z) &&
			equal(v1.w, v2.w)
			;
	}

	template <class T, class U>
	T round(U d)
	{
		BOOST_STATIC_ASSERT(boost::is_floating_point<U>::value);
		BOOST_STATIC_ASSERT(boost::is_integral<T>::value);
		return (T)(d+0.5);
	}

	template <class T, class U>
	void round(T& t, U d)
	{
		BOOST_STATIC_ASSERT(boost::is_floating_point<U>::value);
		BOOST_STATIC_ASSERT(boost::is_integral<T>::value);
		t = (T)(d+0.5);
	}

	template <class T>
	T clamp(T v, T minv, T maxv)
	{
		BOOST_STATIC_ASSERT(boost::is_arithmetic<T>::value);
		custom_assert(minv <= maxv, "");

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
		custom_assert(val > 0, "");

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

	inline float fast_ceil(float val)
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
		if (n.f < val)
		{
			n.f += 1;
		}

		return n.f;
	}

	inline float fast_floor(float val)
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
		if (n.f > val)
		{
			n.f -= 1;
		}

		return n.f;
	}

	// From http://www.stereopsis.com/sree/fpu2006.html
	inline int fast_floori(double d)
	{
		const double DOUBLE_MAGIC_DELTA = 1.5e-8; //almost .5f = .5f + 1e^(number of exp bit)
		const double DOUBLE_MAGIC_ROUND_EPS = 0.5 - DOUBLE_MAGIC_DELTA; //almost .5f = .5f - 1e^(number of exp bit) 
		const double DME = -DOUBLE_MAGIC_ROUND_EPS;
		const double DOUBLE_MAGIC = 6755399441055744.0; // 2^51 + 2^52
		if (d < 0)
		{
			d -= DME;
		}
		else
		{
			d += DME;
		}

		union INTORDOUBLE
		{
			int i;
			double d;
		};

		INTORDOUBLE n;
		n.d = d + DOUBLE_MAGIC;
		return n.i;
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

	vec3 refract3(const vec3& n , const vec3& i , float eta );
	vec4 refract4(const vec4& n , const vec4& i , float eta);

	//v1,v2,v3逆时针旋转
	vec4& gen_plane(vec4& out, const vec4& v1, const vec4& v2, const vec4& v3);
	vec4& hermite(vec4& out, const vec4& v0, const vec4& v1, const vec4& v2, const vec4& v3);
	vec4& cutmull_rom(vec4& out, const vec4& v0, const vec4& v1, const vec4& v2, const vec4& v3);

	//////////////////////////////////////////////
	//  blas level 2: matrix - vector
	//////////////////////////////////////////////
	vec4& transform(vec4& out, const mat44& m, const vec4& v); //v是一个列向量
	vec4& transform_coord(vec4& out, const mat44& m, const vec4& v);
	vec4& transform_normal(vec4& out, const mat44& m, const vec4& v);
	vec4& transform33(vec4& out, const mat44& m, const vec4& v);

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

	mat44& mat_lookat(mat44& out, const vec4& eye, const vec4& target, const vec4& up);

	mat44& mat_projection(mat44& out, float l, float r, float b, float t, float n, float f);
	mat44& mat_perspective(mat44& out, float w, float h, float n, float f);
	mat44& mat_perspective_fov(mat44& out, float fovy, float aspect, float n, float f);
	mat44& mat_ortho(mat44& out, float l, float r, float b, float t, float n, float f);


	/////////////////////////////////////////////
	//  quaternions
	/////////////////////////////////////////////
}
#endif