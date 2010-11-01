#ifndef SOFTART_COLOR_H
#define SOFTART_COLOR_H

/*****************************************************************
*  该文件定义了颜色类与基本的颜色变换、分量获取等操作。
*****************************************************************/

#include "eflib/include/eflib.h"
#include <boost/type_traits.hpp>
#include "softart_fwd.h"
BEGIN_NS_SOFTART()

using eflib::round;

//内部使用的标准颜色类型，实质上是rgba32f。色彩范围没有归一化。
struct color_rgba32f
{
	typedef float component_type;
	static const int component = 4;

	float r, g, b, a;

	color_rgba32f(){}
	color_rgba32f(float r, float g, float b, float a) : r(r), g(g), b(b), a(a){}
	explicit color_rgba32f(const float* color):r(color[0]), g(color[1]), b(color[2]), a(color[3]){}
	explicit color_rgba32f(const eflib::vec4& v):r(v.x), g(v.y), b(v.z), a(v.w){}

	color_rgba32f& operator = (const color_rgba32f& c)
	{
		if(&c == this) return *this;
		r = c.r; g = c.g; b = c.b; a = c.a;
		return *this;
	}

	template <class T>
	color_rgba32f& operator = (const T& c)
	{
		*this = c.to_rgba32f();
		return *this;
	}

	eflib::float4* get_pointer()
	{
		EFLIB_ASSERT(is_aligned(this, 16), "");
		return (eflib::float4*)this;
	}

	const eflib::float4* get_pointer() const
	{
		EFLIB_ASSERT(is_aligned(this, 16), "");
		return (const eflib::float4*)this;
	}

	eflib::vec4& get_vec4()
	{
		return (eflib::vec4&)(*this);
	}

	const eflib::vec4& get_vec4() const
	{
		return (const eflib::vec4&)(*this);
	}
};

struct color_rgb32f
{
	typedef float comp_t;
	comp_t r,g,b;

	color_rgb32f(){}
	explicit color_rgb32f(const comp_t* color):r(color[0]), g(color[1]), b(color[2]){}
	color_rgb32f(comp_t r, comp_t g, comp_t b):r(r), g(g), b(b){}

	template<class T>
	color_rgb32f(const T& rhs){
		*this = rhs;
	}

	color_rgb32f& operator = (const color_rgb32f& rhs){
		r = rhs.r;
		g = rhs.g;
		b = rhs.b;
		return *this;
	}

	color_rgb32f& operator = (const color_rgba32f& rhs){
		return assign(rhs);
	}

	template<class T>
	color_rgb32f& operator = (const T& rhs){
		return assign(rhs.to_rgba32f());
	}

	color_rgba32f to_rgba32f() const{
		return color_rgba32f(r, g, b, 1.0f);
	}

private:
	color_rgb32f& assign(const color_rgba32f& rhs){
		r = rhs.r;
		g = rhs.g;
		b = rhs.b;
		return *this;
	}
};

struct color_r32f
{
	typedef float comp_t;
	comp_t r;

	color_r32f(){}
	explicit color_r32f(const comp_t* color):r(*color){}
	explicit color_r32f(comp_t r):r(r){}

	template<class T>
	color_r32f(const T& rhs){
		*this = rhs;
	}

	color_r32f& operator = (const color_r32f& rhs){
		r = rhs.r;
		return *this;
	}

	color_r32f& operator = (const color_rgba32f& rhs){
		return assign(rhs);
	}

	template<class T>
	color_r32f& operator = (const T& rhs){
		return assign(rhs.to_rgba32f());
	}

	color_rgba32f to_rgba32f() const{
		return color_rgba32f(r, 0.0f, 0.0f, 0.0f);
	}

private:
	color_r32f& assign(const color_rgba32f& rhs){
		r = rhs.r;
		return *this;
	}
};

struct color_rg32f
{
	typedef float comp_t;
	comp_t r, g;

	color_rg32f(){}
	explicit color_rg32f(const comp_t* color):r(color[0]), g(color[1]){}
	color_rg32f(comp_t r, comp_t g):r(r), g(g){}

	template<class T>
	color_rg32f(const T& rhs){
		*this = rhs;
	}

	color_rg32f& operator = (const color_rg32f& rhs){
		r = rhs.r;
		g = rhs.g;
		return *this;
	}

	color_rg32f& operator = (const color_rgba32f& rhs){
		return assign(rhs);
	}

	template<class T>
	color_rg32f& operator = (const T& rhs){
		return assign(rhs.to_rgba32f());
	}

	color_rgba32f to_rgba32f() const{
		return color_rgba32f(r, g, 0.0f, 0.0f);
	}

private:
	color_rg32f& assign(const color_rgba32f& rhs){
		r = rhs.r;
		g = rhs.g;
		return *this;
	}
};

/***********************************************************
* 以下类型为四分量的颜色，每个颜色为8位整型。
**********************************************************/
struct color_rgba8
{
	typedef uint8_t comp_t;
	comp_t r, g, b, a;

	color_rgba8(){}
	explicit color_rgba8(const comp_t* color):r(color[0]), g(color[1]), b(color[2]), a(color[3]){}
	color_rgba8(comp_t r, comp_t g, comp_t b, comp_t a):r(r), g(g), b(b), a(a){}

	template<class T>
	color_rgba8(const T& rhs){
		*this = rhs;
	}

	color_rgba8& operator = (const color_rgba8& rhs){
		r = rhs.r; g = rhs.g;	b = rhs.b; a = rhs.a;
		return *this;
	}

	color_rgba8& operator = (const color_rgba32f& rhs){
		return assign(rhs);
	}

	template <class T>
	color_rgba8& operator = (const T& rhs){
		return assign(rhs.to_rgba32f());
	}

	color_rgba32f to_rgba32f() const{
		const float inv_255 = 1.0f / 255;
		return color_rgba32f(r * inv_255, g * inv_255, b * inv_255, a * inv_255);
	}
private:
	color_rgba8& assign(const color_rgba32f& rhs){
#ifndef EFLIB_NO_SIMD
		const __m128 f255 = _mm_set_ps1(255.0f);
		__m128 m4 = _mm_loadu_ps(&rhs.r);
		m4 = _mm_mul_ps(m4, f255);
		m4 = _mm_max_ps(m4, _mm_setzero_ps());
		m4 = _mm_min_ps(m4, f255);
		__m128i mi4 = _mm_cvtps_epi32(m4);
		mi4 = _mm_or_si128(mi4, _mm_srli_si128(mi4, 3));
		mi4 = _mm_or_si128(mi4, _mm_srli_si128(mi4, 6));
		*reinterpret_cast<int*>(&r) = _mm_cvtsi128_si32(mi4);
#else
		r = comp_t( eflib::clamp(rhs.r * 255.0f, 0.0f, 255.0f) );
		g = comp_t( eflib::clamp(rhs.g * 255.0f, 0.0f, 255.0f) );
		b = comp_t( eflib::clamp(rhs.b * 255.0f, 0.0f, 255.0f) );
		a = comp_t( eflib::clamp(rhs.a * 255.0f, 0.0f, 255.0f) );
#endif

		return *this;
	}
};

struct color_bgra8
{
	typedef uint8_t comp_t;
	comp_t b, g, r, a;

	color_bgra8(){}
	explicit color_bgra8(const comp_t* color):b(color[0]), g(color[1]), r(color[2]), a(color[3]){}
	color_bgra8(comp_t b, comp_t g, comp_t r, comp_t a):b(b), g(g), r(r), a(a){}

	template<class T>
	color_bgra8(const T& rhs){
		*this = rhs;
	}

	color_bgra8& operator = (const color_bgra8& rhs){
		r = rhs.r; g = rhs.g;	b = rhs.b; a = rhs.a;
		return *this;
	}

	color_bgra8& operator = (const color_rgba32f& rhs){
		return assign(rhs);
	}

	template <class T>
	color_bgra8& operator = (const T& rhs){
		return assign(rhs.to_rgba32f());
	}

	color_rgba32f to_rgba32f() const{
		const float inv_255 = 1.0f / 255;
		return color_rgba32f(r * inv_255, g * inv_255, b * inv_255, a * inv_255);
	}
private:
	color_bgra8& assign(const color_rgba32f& rhs){
#ifndef EFLIB_NO_SIMD
		const __m128 f255 = _mm_set_ps1(255.0f);
		__m128 m4 = _mm_loadu_ps(&rhs.r);
		m4 = _mm_shuffle_ps(m4, m4, _MM_SHUFFLE(3, 0, 1, 2));
		m4 = _mm_mul_ps(m4, f255);
		m4 = _mm_max_ps(m4, _mm_setzero_ps());
		m4 = _mm_min_ps(m4, f255);
		__m128i mi4 = _mm_cvtps_epi32(m4);
		mi4 = _mm_or_si128(mi4, _mm_srli_si128(mi4, 3));
		mi4 = _mm_or_si128(mi4, _mm_srli_si128(mi4, 6));
		*reinterpret_cast<int*>(&b) = _mm_cvtsi128_si32(mi4);
#else
		r = comp_t( eflib::clamp(rhs.r * 255.0f + 0.5f, 0.0f, 255.0f) );
		g = comp_t( eflib::clamp(rhs.g * 255.0f + 0.5f, 0.0f, 255.0f) );
		b = comp_t( eflib::clamp(rhs.b * 255.0f + 0.5f, 0.0f, 255.0f) );
		a = comp_t( eflib::clamp(rhs.a * 255.0f + 0.5f, 0.0f, 255.0f) );
#endif

		return *this;
	}
};

struct color_r32i
{
	typedef int32_t comp_t;
	comp_t r;

	color_r32i(){}
	explicit color_r32i(const comp_t* color):r(*color){}
	color_r32i(comp_t r):r(r){}

	template<class T>
	color_r32i(const T& rhs){
		*this = rhs;
	}

	color_r32i& operator = (const color_r32i& rhs){
		r = rhs.r;
		return *this;
	}

	color_r32i& operator = (const color_rgba32f& rhs){
		return assign(rhs);
	}

	template<class T>
	color_r32i& operator = (const T& rhs){
		return assign(rhs.to_rgba32f());
	}

	color_rgba32f to_rgba32f() const{
		return color_rgba32f(float(r), 0.0f, 0.0f, 0.0f);
	}

private:
	color_r32i& assign(const color_rgba32f& rhs){
		r = comp_t( rhs.r + 0.5f );
		return *this;
	}
};

inline color_rgba32f lerp(const color_rgba32f& c0, const color_rgba32f& c1, float t)
{
#ifndef EFLIB_NO_SIMD
	__m128 mc0 = _mm_loadu_ps(&c0.r);
	__m128 mc1 = _mm_loadu_ps(&c1.r);
	__m128 mret = _mm_add_ps(mc0, _mm_mul_ps(_mm_sub_ps(mc1, mc0), _mm_set1_ps(t)));
	color_rgba32f ret;
	_mm_storeu_ps(&ret.r, mret);
	return ret;
#else
	return color_rgba32f(c0.get_vec4() + (c1.get_vec4() - c0.get_vec4()) * t);
#endif
}
inline color_rgba32f lerp(const color_rgb32f& c0, const color_rgb32f& c1, float t)
{
	return color_rgb32f(c0.r + (c1.r - c0.r) * t, c0.r + (c1.g - c0.g) * t, c0.r + (c1.b - c0.b) * t).to_rgba32f();
}
inline color_rgba32f lerp(const color_bgra8& c0, const color_bgra8& c1, float t)
{
	color_rgba32f ret = lerp(color_rgba32f(c0.r, c0.g, c0.b, c0.a), color_rgba32f(c1.r, c1.g, c1.b, c1.a), t);
	ret.get_vec4() /= 255.0f;
	return ret;
}
inline color_rgba32f lerp(const color_rgba8& c0, const color_rgba8& c1, float t)
{
	color_rgba32f ret = lerp(color_rgba32f(c0.r, c0.g, c0.b, c0.a), color_rgba32f(c1.r, c1.g, c1.b, c1.a), t);
	ret.get_vec4() /= 255.0f;
	return ret;
}
inline color_rgba32f lerp(const color_r32f& c0, const color_r32f& c1, float t)
{
	return color_r32f(c0.r + (c1.r - c0.r) * t).to_rgba32f();
}
inline color_rgba32f lerp(const color_rg32f& c0, const color_rg32f& c1, float t)
{
	return color_rg32f(c0.r + (c1.r - c0.r) * t, c0.r + (c1.g - c0.g) * t).to_rgba32f();
}
inline color_rgba32f lerp(const color_r32i& c0, const color_r32i& c1, float t)
{
	return color_r32i(static_cast<color_r32i::comp_t>(c0.r + (c1.r - c0.r) * t)).to_rgba32f();
}

inline color_rgba32f lerp(const color_rgba32f& c0, const color_rgba32f& c1, const color_rgba32f& c2, const color_rgba32f& c3, float tx, float ty)
{
#ifndef EFLIB_NO_SIMD
	__m128 mc0 = _mm_loadu_ps(&c0.r);
	__m128 mc1 = _mm_loadu_ps(&c1.r);
	__m128 mc2 = _mm_loadu_ps(&c2.r);
	__m128 mc3 = _mm_loadu_ps(&c3.r);
	__m128 mc01 = _mm_add_ps(mc0, _mm_mul_ps(_mm_sub_ps(mc1, mc0), _mm_set1_ps(tx)));
	__m128 mc23 = _mm_add_ps(mc2, _mm_mul_ps(_mm_sub_ps(mc3, mc2), _mm_set1_ps(tx)));
	__m128 mret = _mm_add_ps(mc01, _mm_mul_ps(_mm_sub_ps(mc23, mc01), _mm_set1_ps(ty)));
	color_rgba32f ret;
	_mm_storeu_ps(&ret.r, mret);
	return ret;
#else
	color_rgba32f c01(c0.get_vec4() + (c1.get_vec4() - c0.get_vec4()) * tx);
	color_rgba32f c23(c2.get_vec4() + (c3.get_vec4() - c2.get_vec4()) * tx);
	return color_rgba32f(c01.get_vec4() + (c23.get_vec4() - c01.get_vec4()) * ty);
#endif
}
inline color_rgba32f lerp(const color_rgb32f& c0, const color_rgb32f& c1, const color_rgb32f& c2, const color_rgb32f& c3, float tx, float ty)
{
	color_rgb32f c01(c0.r + (c1.r - c0.r) * tx, c0.r + (c1.g - c0.g) * tx, c0.r + (c1.b - c0.b) * tx);
	color_rgb32f c23(c2.r + (c3.r - c2.r) * tx, c2.r + (c3.g - c2.g) * tx, c2.r + (c3.b - c2.b) * tx);
	return color_rgb32f(c01.r + (c23.r - c01.r) * ty, c01.r + (c23.g - c01.g) * ty, c01.r + (c23.b - c01.b) * ty).to_rgba32f();
}
inline color_rgba32f lerp(const color_bgra8& c0, const color_bgra8& c1, const color_bgra8& c2, const color_bgra8& c3, float tx, float ty)
{
#ifndef EFLIB_NO_SIMD
	__m128i mzero = _mm_setzero_si128();
	__m128i mci = _mm_cvtsi32_si128(*reinterpret_cast<const int*>(&c0.r));
	mci = _mm_unpacklo_epi8(mci, mzero);
	mci = _mm_unpacklo_epi16(mci, mzero);
	__m128 mc0 = _mm_cvtepi32_ps(_mm_shuffle_epi32(mci, _MM_SHUFFLE(3, 0, 1, 2)));
	mci = _mm_cvtsi32_si128(*reinterpret_cast<const int*>(&c1.r));
	mci = _mm_unpacklo_epi8(mci, mzero);
	mci = _mm_unpacklo_epi16(mci, mzero);
	__m128 mc1 = _mm_cvtepi32_ps(_mm_shuffle_epi32(mci, _MM_SHUFFLE(3, 0, 1, 2)));
	mci = _mm_cvtsi32_si128(*reinterpret_cast<const int*>(&c2.r));
	mci = _mm_unpacklo_epi8(mci, mzero);
	mci = _mm_unpacklo_epi16(mci, mzero);
	__m128 mc2 = _mm_cvtepi32_ps(_mm_shuffle_epi32(mci, _MM_SHUFFLE(3, 0, 1, 2)));
	mci = _mm_cvtsi32_si128(*reinterpret_cast<const int*>(&c3.r));
	mci = _mm_unpacklo_epi8(mci, mzero);
	mci = _mm_unpacklo_epi16(mci, mzero);
	__m128 mc3 = _mm_cvtepi32_ps(_mm_shuffle_epi32(mci, _MM_SHUFFLE(3, 0, 1, 2)));

	__m128 mc01 = _mm_add_ps(mc0, _mm_mul_ps(_mm_sub_ps(mc1, mc0), _mm_set1_ps(tx)));
	__m128 mc23 = _mm_add_ps(mc2, _mm_mul_ps(_mm_sub_ps(mc3, mc2), _mm_set1_ps(tx)));
	__m128 mret = _mm_add_ps(mc01, _mm_mul_ps(_mm_sub_ps(mc23, mc01), _mm_set1_ps(ty)));
	mret = _mm_mul_ps(mret, _mm_set1_ps(1.0f / 255));
	color_rgba32f ret;
	_mm_storeu_ps(&ret.r, mret);
	return ret;
#else
	color_rgba32f ret = lerp(color_rgba32f(c0.r, c0.g, c0.b, c0.a), color_rgba32f(c1.r, c1.g, c1.b, c1.a),
							color_rgba32f(c2.r, c2.g, c2.b, c2.a), color_rgba32f(c3.r, c3.g, c3.b, c3.a), tx, ty);
	ret.get_vec4() /= 255.0f;
	return ret;
#endif
}
inline color_rgba32f lerp(const color_rgba8& c0, const color_rgba8& c1, const color_rgba8& c2, const color_rgba8& c3, float tx, float ty)
{
#ifndef EFLIB_NO_SIMD
	__m128i mzero = _mm_setzero_si128();
	__m128i mci = _mm_cvtsi32_si128(*reinterpret_cast<const int*>(&c0.r));
	mci = _mm_unpacklo_epi8(mci, mzero);
	__m128 mc0 = _mm_cvtepi32_ps(_mm_unpacklo_epi16(mci, mzero));
	mci = _mm_cvtsi32_si128(*reinterpret_cast<const int*>(&c1.r));
	mci = _mm_unpacklo_epi8(mci, mzero);
	__m128 mc1 = _mm_cvtepi32_ps(_mm_unpacklo_epi16(mci, mzero));
	mci = _mm_cvtsi32_si128(*reinterpret_cast<const int*>(&c2.r));
	mci = _mm_unpacklo_epi8(mci, mzero);
	__m128 mc2 = _mm_cvtepi32_ps(_mm_unpacklo_epi16(mci, mzero));
	mci = _mm_cvtsi32_si128(*reinterpret_cast<const int*>(&c3.r));
	mci = _mm_unpacklo_epi8(mci, mzero);
	__m128 mc3 = _mm_cvtepi32_ps(_mm_unpacklo_epi16(mci, mzero));

	__m128 mc01 = _mm_add_ps(mc0, _mm_mul_ps(_mm_sub_ps(mc1, mc0), _mm_set1_ps(tx)));
	__m128 mc23 = _mm_add_ps(mc2, _mm_mul_ps(_mm_sub_ps(mc3, mc2), _mm_set1_ps(tx)));
	__m128 mret = _mm_add_ps(mc01, _mm_mul_ps(_mm_sub_ps(mc23, mc01), _mm_set1_ps(ty)));
	mret = _mm_mul_ps(mret, _mm_set1_ps(1.0f / 255));
	color_rgba32f ret;
	_mm_storeu_ps(&ret.r, mret);
	return ret;
#else
	color_rgba32f ret = lerp(color_rgba32f(c0.r, c0.g, c0.b, c0.a), color_rgba32f(c1.r, c1.g, c1.b, c1.a),
							color_rgba32f(c2.r, c2.g, c2.b, c2.a), color_rgba32f(c3.r, c3.g, c3.b, c3.a), tx, ty);
	ret.get_vec4() /= 255.0f;
	return ret;
#endif
}
inline color_rgba32f lerp(const color_r32f& c0, const color_r32f& c1, const color_r32f& c2, const color_r32f& c3, float tx, float ty)
{
	color_r32f c01(c0.r + (c1.r - c0.r) * tx);
	color_r32f c23(c2.r + (c3.r - c2.r) * tx);
	return color_r32f(c01.r + (c23.r - c01.r) * ty).to_rgba32f();
}
inline color_rgba32f lerp(const color_rg32f& c0, const color_rg32f& c1, const color_rg32f& c2, const color_rg32f& c3, float tx, float ty)
{
	color_rg32f c01(c0.r + (c1.r - c0.r) * tx, c0.r + (c1.g - c0.g) * tx);
	color_rg32f c23(c2.r + (c3.r - c2.r) * tx, c2.r + (c3.g - c2.g) * tx);
	return color_rg32f(c01.r + (c23.r - c01.r) * ty, c01.r + (c23.g - c01.g) * ty).to_rgba32f();
}
inline color_rgba32f lerp(const color_r32i& c0, const color_r32i& c1, const color_r32i& c2, const color_r32i& c3, float tx, float ty)
{
	color_r32f c01(c0.r + (c1.r - c0.r) * tx);
	color_r32f c23(c2.r + (c3.r - c2.r) * tx);
	return color_r32f(c01.r + (c23.r - c01.r) * ty).to_rgba32f();
}

END_NS_SOFTART()

#include "colors_convertors.h"


#endif
