#ifndef SOFTART_COLOR_H
#define SOFTART_COLOR_H

/*****************************************************************
*  该文件定义了颜色类与基本的颜色变换、分量获取等操作。
*****************************************************************/

#include "eflib/include/eflib.h"
#include <boost/type_traits.hpp>
using efl::round;

//内部使用的标准颜色类型，实质上是rgba32f。色彩范围没有归一化。
struct color_rgba32f
{
	typedef float component_type;
	static const int component = 4;

	float r, g, b, a;

	color_rgba32f() : r(0.0f), g(0.0f), b(0.0f), a(0.0f){}
	color_rgba32f(float r, float g, float b, float a) : r(r), g(g), b(b), a(a){}
	color_rgba32f(const float* color):r(color[0]), g(color[1]), b(color[2]), a(color[3]){}
	color_rgba32f(const efl::vec4& v):r(v.x), g(v.y), b(v.z), a(v.w){}

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

	operator float* (){return (float*)this;}
	operator const float* () const {return (const float*)this;}

	efl::float4* get_pointer()
	{
		custom_assert(is_aligned(this, 16), "");
		return (efl::float4*)this;
	}

	const efl::float4* get_pointer() const
	{
		custom_assert(is_aligned(this, 16), "");
		return (const efl::float4*)this;
	}

	efl::vec4& get_vec4()
	{
		return (efl::vec4&)(*this);
	}

	const efl::vec4& get_vec4() const
	{
		return (const efl::vec4&)(*this);
	}
};

struct color_rgb32f
{
	typedef float comp_t;
	comp_t r,g,b;

	color_rgb32f():r(0.0f), g(0.0f), b(0.0f){}
	color_rgb32f(const comp_t* color):r(color[0]), g(color[1]), b(color[2]){}
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

	color_r32f():r(0.0f){}
	color_r32f(const comp_t* color):r(*color){}
	color_r32f(comp_t r):r(r){}

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

	color_rg32f():r(0.0f), g(0.0f){}
	color_rg32f(const comp_t* color):r(color[0]), g(color[1]){}
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

	color_rgba8() : r(0), g(0), b(0), a(0){}
	color_rgba8(const comp_t* color):r(color[0]), g(color[1]), b(color[2]), a(color[3]){}
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
		return color_rgba32f(r/255.0f, g / 255.0f, b/255.0f, a/255.0f);
	}
private:
	color_rgba8& assign(const color_rgba32f& rhs){
		r = comp_t( efl::clamp(rhs.r * 255.0f, 0.0f, 255.0f) );
		g = comp_t( efl::clamp(rhs.g * 255.0f, 0.0f, 255.0f) );
		b = comp_t( efl::clamp(rhs.b * 255.0f, 0.0f, 255.0f) );
		a = comp_t( efl::clamp(rhs.a * 255.0f, 0.0f, 255.0f) );

		return *this;
	}
};

struct color_bgra8
{
	typedef uint8_t comp_t;
	comp_t b, g, r, a;

	color_bgra8() : r(0), g(0), b(0), a(0){}
	color_bgra8(const comp_t* color):b(color[0]), g(color[1]), r(color[2]), a(color[3]){}
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
		return color_rgba32f(r/255.0f, g / 255.0f, b/255.0f, a/255.0f);
	}
private:
	color_bgra8& assign(const color_rgba32f& rhs){
		r = comp_t( efl::clamp(rhs.r * 255.0f, 0.0f, 255.0f) );
		g = comp_t( efl::clamp(rhs.g * 255.0f, 0.0f, 255.0f) );
		b = comp_t( efl::clamp(rhs.b * 255.0f, 0.0f, 255.0f) );
		a = comp_t( efl::clamp(rhs.a * 255.0f, 0.0f, 255.0f) );
		return *this;
	}
};

template<class T>
T lerp(const T& c0, float t0, const T& c1, float t1)
{
	return T(lerp(c0.to_rgba32f(), t0 + c1.to_rgba32f(), t1));
}

inline color_rgba32f lerp(const color_rgba32f& c0, float t0, const color_rgba32f& c1, float t1)
{
	return color_rgba32f(c0.get_vec4() * t0 + c1.get_vec4() * t1);
}

#include "colors_convertors.h"

#endif