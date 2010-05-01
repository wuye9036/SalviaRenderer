#ifndef EFLIB_VECTOR_H
#define EFLIB_VECTOR_H

#include "swizzle.h"
#include "write_mask.h"

#include "../platform.h"
#include "../debug_helper.h"
#include <cmath>

#ifdef EFLIB_MSVC
#   include <xutility>
#endif

namespace efl{

#ifndef EFLIB_NO_SIMD
	typedef ALIGN16 float float4[4];
	typedef float4 float4x4[4];
#endif

	struct vec2;
	struct vec3;
	struct vec4;

	struct int2;
	struct int3;
	struct int4;

	struct vec2
	{
		float x, y;

		/////////////////////////////////////
		//		标准迭代器
		/////////////////////////////////////

		typedef const float*  const_iterator;
		typedef float* iterator;

		const_iterator begin() const
		{
			return (const_iterator)this;
		}

		const_iterator end() const
		{
			return ((const_iterator)this) + 2;
		}

		iterator begin()
		{
			return (iterator)this;
		}

		iterator end()
		{
			return ((iterator)this) + 2;
		}

		///////////////////////////////////////
		//	类型转换
		///////////////////////////////////////
		float* ptr()
		{
			return (float*)this;
		}

		const float* ptr() const
		{
			return (const float*)this;
		}

		///////////////////////////////////////////////
		//	构造函数
		///////////////////////////////////////////////
		vec2(){}
		vec2(float x, float y) : x(x), y(y){}
		vec2(const vec2& v) : x(v.x), y(v.y){}

		SWIZZLE_DECL_FOR_VEC2();
		WRITE_MASK_FOR_VEC2();

		const float& operator [](size_t i) const
		{
			custom_assert(i < 1, "");
			return ((float*)(this))[i];
		}

		float& operator [](size_t i)
		{
			custom_assert(i < 1, "");
			return ((float*)(this))[i];
		}

		//////////////////////////////////////////////////
		//	四则运算
		/////////////////////////////////////////////////
		vec2 operator - () const
		{
			return vec2(-x, -y);
		}

		vec2& operator *= (float s)
		{
			x *= s;
			y *= s;
			return *this;
		}

		vec2& operator /= (float s)
		{
			float invs = 1 / s;
			return ((*this) *= invs);
		}

		vec2& operator += (float s)
		{
			x += s;
			y += s;
			return *this;
		}

		vec2& operator -= (float s)
		{
			x -= s;
			y -= s;
			return *this;
		}

		/////////////////////////////////////////////
		//	其他运算
		/////////////////////////////////////////////
		float length_sqr() const
		{
			return x*x + y*y;
		}

		float length() const
		{
			return std::sqrt(length_sqr());
		}

		void normalize()
		{
			float len = length();
			(*this) /= len;
		}

	private:
		bool operator == (const vec2& rhs);
		bool operator != (const vec2& rhs);
	};

	struct vec3
	{
		float x, y, z;

		/////////////////////////////////////
		//		标准迭代器
		/////////////////////////////////////
		typedef const float*  const_iterator;
		typedef float* iterator;

		const_iterator begin() const
		{
			return (const_iterator)this;
		}

		const_iterator end() const
		{
			return ((const_iterator)this) + 3;
		}

		iterator begin()
		{
			return (iterator)this;
		}

		iterator end()
		{
			return ((iterator)this) + 3;
		}

		///////////////////////////////////////
		//	类型转换
		///////////////////////////////////////
		float* ptr()
		{
			return (float*)this;
		}

		const float* ptr() const
		{
			return (const float*)this;
		}

		const float& operator [](size_t i) const
		{
			custom_assert(i < 2, "");
			return ((float*)(this))[i];
		}

		float& operator [](size_t i)
		{
			custom_assert(i < 2, "");
			return ((float*)(this))[i];
		}

		SWIZZLE_DECL_FOR_VEC3();
		WRITE_MASK_FOR_VEC3();

		///////////////////////////////////////////////
		//	构造函数
		///////////////////////////////////////////////
		vec3(){}
		vec3(float x, float y, float z) : x(x), y(y), z(z){}
		vec3(const vec3& v) : x(v.x), y(v.y), z(v.z){}
		//////////////////////////////////////////////////
		//	四则运算
		/////////////////////////////////////////////////
		vec3 operator - () const
		{
			return vec3(-x, -y, -z);
		}

		vec3& operator *= (float s)
		{
			x *= s;
			y *= s;
			z *= s;
			return *this;
		}

		vec3& operator /= (float s)
		{
			float invs = 1 / s;
			return ((*this) *= invs);
		}

		vec3& operator += (float s)
		{
			x += s;
			y += s;
			z += s;
			return *this;
		}

		vec3& operator -= (float s)
		{
			x -= s;
			y -= s;
			z -= s;
			return *this;
		}

		/////////////////////////////////////////////
		//	其他运算
		/////////////////////////////////////////////
		float length_sqr() const
		{
			return x*x + y*y + z*z;
		}

		float length() const
		{
			return std::sqrt(length_sqr());
		}

		void normalize()
		{
			float len = length();
			(*this) /= len;
		}
	private:
		bool operator == (const vec3& rhs);
		bool operator != (const vec3& rhs);
	};

	struct vec4
	{
#ifdef EFLIB_MSVC
#pragma warning(push)
#pragma warning(disable : 4201)
#endif
		union{
			struct {
				float x, y, z, w;
			};
			struct {
				float s, t, r, q;
			};
		};
#ifdef EFLIB_MSVC
#pragma warning(pop)
#endif

		/////////////////////////////////////
		//		标准迭代器
		/////////////////////////////////////
		typedef const float*  const_iterator;
		typedef float* iterator;

		const_iterator begin() const
		{
			return (const_iterator)this;
		}

		const_iterator end() const
		{
			return ((const_iterator)this) + 4;
		}

		iterator begin()
		{
			return (iterator)this;
		}

		iterator end()
		{
			return ((iterator)this) + 4;
		}

		///////////////////////////////////////
		//	类型转换
		///////////////////////////////////////
		float* ptr()
		{
			return (float*)this;
		}

		const float* ptr() const
		{
			return (const float*)this;
		}

		SWIZZLE_DECL_FOR_VEC4();
		WRITE_MASK_FOR_VEC4();

		const float& operator [](size_t i) const
		{
			custom_assert(i < 3, "");
			return ((float*)(this))[i];
		}

		float& operator [](size_t i)
		{
			custom_assert(i < 3, "");
			return ((float*)(this))[i];
		}
		///////////////////////////////////////////////
		//	构造函数
		///////////////////////////////////////////////
		vec4(){}

		explicit vec4(float x, float y = 0.0f, float z = 0.0f, float w = 0.0f):x(x), y(y), z(z), w(w){}
		
		vec4(const vec4& v) : x(v.x), y(v.y), z(v.z), w(v.w){}
		
		vec4(const vec3& v, float w): x(v.x), y(v.y), z(v.z), w(w){}

		vec4(const float* fv, int sizefv)
		{
			int clamped_size = std::max(sizefv, 4);
			for(int i = 0; i < clamped_size; ++i) 
				(*this)[i] = fv[i];
		}

		//////////////////////////////////////////////////
		//	四则运算。乘除为逐元素运算
		/////////////////////////////////////////////////
		vec4 operator - () const
		{
			return vec4(-x, -y, -z, -w);
		}

		vec4& operator += (const vec4& v)
		{
			x += v.x;
			y += v.y;
			z += v.z;
			w += v.w;
			return *this;
		}

		vec4& operator -= (const vec4& v)
		{
			x -= v.x;
			y -= v.y;
			z -= v.z;
			w -= v.w;
			return *this;
		}

		vec4& operator *= (const vec4& v)
		{
			x *= v.x;
			y *= v.y;
			z *= v.z;
			w *= v.w;
			return *this;
		}

		vec4& operator /= (const vec4& v)
		{
			x /= v.x;
			y /= v.y;
			z /= v.z;
			w /= v.w;
			return *this;
		}

		vec4& operator *= (float s)
		{
			x *= s;
			y *= s;
			z *= s;
			w *= s;
			return *this;
		}

		vec4& operator /= (float s)
		{
			float invs = 1 / s;
			return ((*this) *= invs);
		}

		vec4& operator += (float s)
		{
			x += s;
			y += s;
			z += s;
			w += s;
			return *this;
		}

		vec4& operator -= (float s)
		{
			x -= s;
			y -= s;
			z -= s;
			w -= s;
			return *this;
		}

		/////////////////////////////////////////////
		//	其他运算
		/////////////////////////////////////////////
		float length_sqr() const
		{
			return x*x + y*y + z*z + w*w;
		}

		float length() const
		{
			return std::sqrt(length_sqr());
		}

		void normalize()
		{
			float len = length();
			(*this) /= len;
		}

		void normalize3()
		{
			this->xyz().normalize();
		}

		void projection()
		{
			xyz() /= w;
			w = 1.0f;
		}

		//static function
		static vec4 zero(){
			return vec4(0.0f, 0.0f, 0.0f, 0.0f);
		}

		static vec4 gen_coord(float x, float y, float z){
			return vec4(x, y, z, 1.0f);
		}

		static vec4 gen_vector(float x, float y, float z){
			return vec4(x, y, z, 0.0f);
		}

	private:
		bool operator == (const vec4& rhs);
		bool operator != (const vec4& rhs);
	};
	
	typedef vec4 coord;

#ifdef EFLIB_NO_SIMD
	typedef vec4 float4;
#endif

	//non-member non-friend

	//vec2
	inline vec2 operator + (const vec2& lhs, const vec2& rhs){
		return vec2(lhs.x+rhs.x, lhs.y+rhs.y);
	}

	inline vec2 operator - (const vec2& lhs, const vec2& rhs){
		return vec2(lhs.x-rhs.x, lhs.y-rhs.y);
	}

	inline vec2 operator + (const vec2& lhs, float s){
		return vec2(lhs.x+s, lhs.y+s);
	}

	inline vec2 operator + (float s, const vec2& lhs){
		return lhs + s;
	}

	inline vec2 operator - (const vec2& lhs, float s){
		return vec2(lhs.x-s, lhs.y-s);
	}

	inline vec2 operator - (float s, const vec2& lhs){
		return lhs - s;
	}

	inline vec2 operator * (const vec2& lhs, float s){
		return vec2(lhs.x*s, lhs.y*s);
	}

	inline vec2 operator * (float s, const vec2& lhs){
		return lhs * s;
	}

	inline vec2 operator * (const vec2& lhs, const vec2& rhs){
		return vec2(lhs.x * rhs.x, lhs.y * rhs.y);
	}

	inline vec2 operator / (const vec2& lhs, float s){
		float invs = 1.0f / s;
		return lhs * invs;
	}

	inline vec2 operator / (const vec2& lhs, const vec2& rhs){
		return vec2(lhs.x / rhs.x, lhs.y / rhs.y);
	}

	//vec3
	inline vec3 operator + (const vec3& lhs, const vec3& rhs){
		return vec3(lhs.x+rhs.x, lhs.y+rhs.y, lhs.z+rhs.z);
	}

	inline vec3 operator - (const vec3& lhs, const vec3& rhs){
		return vec3(lhs.x-rhs.x, lhs.y-rhs.y, lhs.z-rhs.z);
	}

	inline vec3 operator + (const vec3& lhs, float s){
		return vec3(lhs.x+s, lhs.y+s, lhs.z+s);
	}

	inline vec3 operator + (float s, const vec3& lhs){
		return lhs + s;
	}

	inline vec3 operator - (const vec3& lhs, float s){
		return vec3(lhs.x-s, lhs.y-s, lhs.z-s);
	}

	inline vec3 operator - (float s, const vec3& lhs){
		return lhs - s;
	}

	inline vec3 operator * (const vec3& lhs, float s){
		return vec3(lhs.x*s, lhs.y*s, lhs.z*s);
	}

	inline vec3 operator * (float s, const vec3& lhs){
		return lhs * s;
	}

	inline vec3 operator * (const vec3& lhs, const vec3& rhs){
		return vec3(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z);
	}

	inline vec3 operator / (const vec3& lhs, float s){
		float invs = 1.0f / s;
		return lhs * invs;
	}

	inline vec3 operator / (const vec3& lhs, const vec3& rhs){
		return vec3(lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z);
	}

	//vec4
	inline vec4 operator + (const vec4& lhs, const vec4& rhs){
		return vec4(lhs.x+rhs.x, lhs.y+rhs.y, lhs.z+rhs.z, lhs.w+rhs.w);
	}

	inline vec4 operator - (const vec4& lhs, const vec4& rhs){
		return vec4(lhs.x-rhs.x, lhs.y-rhs.y, lhs.z-rhs.z, lhs.w-rhs.w);
	}

	inline vec4 operator + (const vec4& lhs, float s){
		return vec4(lhs.x+s, lhs.y+s, lhs.z+s, lhs.w+s);
	}

	inline vec4 operator + (float s, const vec4& lhs){
		return lhs + s;
	}

	inline vec4 operator - (const vec4& lhs, float s){
		return vec4(lhs.x-s, lhs.y-s, lhs.z-s, lhs.w-s);
	}

	inline vec4 operator - (float s, const vec4& lhs){
		return lhs - s;
	}

	inline vec4 operator * (const vec4& lhs, float s){
		return vec4(lhs.x*s, lhs.y*s, lhs.z*s, lhs.w*s);
	}

	inline vec4 operator * (float s, const vec4& lhs){
		return lhs * s;
	}

	inline vec4 operator * (const vec4& lhs, const vec4& rhs){
		return vec4(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z, lhs.w * rhs.w);
	}

	inline vec4 operator / (const vec4& lhs, float s){
		float invs = 1.0f / s;
		return lhs * invs;
	}

	inline vec4 operator / (const vec4& lhs, const vec4& rhs){
		return vec4(lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z, lhs.w / rhs.w);
	}

	SWIZZLE_IMPL_FOR_VEC2();
	SWIZZLE_IMPL_FOR_VEC3();
	SWIZZLE_IMPL_FOR_VEC4();


	struct int2
	{
		int x, y;

		/////////////////////////////////////
		//		标准迭代器
		/////////////////////////////////////

		typedef const int*  const_iterator;
		typedef int* iterator;

		const_iterator begin() const
		{
			return (const_iterator)this;
		}

		const_iterator end() const
		{
			return ((const_iterator)this) + 2;
		}

		iterator begin()
		{
			return (iterator)this;
		}

		iterator end()
		{
			return ((iterator)this) + 2;
		}

		///////////////////////////////////////
		//	类型转换
		///////////////////////////////////////
		int* ptr()
		{
			return (int*)this;
		}

		const int* ptr() const
		{
			return (const int*)this;
		}

		///////////////////////////////////////////////
		//	构造函数
		///////////////////////////////////////////////
		int2(){}
		int2(int x, int y) : x(x), y(y){}
		int2(const int2& v) : x(v.x), y(v.y){}

		const int& operator [](size_t i) const
		{
			custom_assert(i < 1, "");
			return ((int*)(this))[i];
		}

		int& operator [](size_t i)
		{
			custom_assert(i < 1, "");
			return ((int*)(this))[i];
		}

		//////////////////////////////////////////////////
		//	四则运算
		/////////////////////////////////////////////////
		int2 operator - () const
		{
			return int2(-x, -y);
		}

		int2& operator *= (int s)
		{
			x *= s;
			y *= s;
			return *this;
		}

		int2& operator /= (int s)
		{
			int invs = 1 / s;
			return ((*this) *= invs);
		}

		int2& operator += (int s)
		{
			x += s;
			y += s;
			return *this;
		}

		int2& operator -= (int s)
		{
			x -= s;
			y -= s;
			return *this;
		}

		/////////////////////////////////////////////
		//	其他运算
		/////////////////////////////////////////////
		int length_sqr() const
		{
			return x*x + y*y;
		}

		float length() const
		{
			return std::sqrt(static_cast<float>(length_sqr()));
		}

		void normalize()
		{
			int len = static_cast<int>(length());
			(*this) /= len;
		}

	private:
		bool operator == (const int2& rhs);
		bool operator != (const int2& rhs);
	};

	struct int3
	{
		int x, y, z;

		/////////////////////////////////////
		//		标准迭代器
		/////////////////////////////////////
		typedef const int*  const_iterator;
		typedef int* iterator;

		const_iterator begin() const
		{
			return (const_iterator)this;
		}

		const_iterator end() const
		{
			return ((const_iterator)this) + 3;
		}

		iterator begin()
		{
			return (iterator)this;
		}

		iterator end()
		{
			return ((iterator)this) + 3;
		}

		///////////////////////////////////////
		//	类型转换
		///////////////////////////////////////
		int* ptr()
		{
			return (int*)this;
		}

		const int* ptr() const
		{
			return (const int*)this;
		}

		const int& operator [](size_t i) const
		{
			custom_assert(i < 2, "");
			return ((int*)(this))[i];
		}

		int& operator [](size_t i)
		{
			custom_assert(i < 2, "");
			return ((int*)(this))[i];
		}

		///////////////////////////////////////////////
		//	构造函数
		///////////////////////////////////////////////
		int3(){}
		int3(int x, int y, int z) : x(x), y(y), z(z){}
		int3(const int3& v) : x(v.x), y(v.y), z(v.z){}
		//////////////////////////////////////////////////
		//	四则运算
		/////////////////////////////////////////////////
		int3 operator - () const
		{
			return int3(-x, -y, -z);
		}

		int3& operator *= (int s)
		{
			x *= s;
			y *= s;
			z *= s;
			return *this;
		}

		int3& operator /= (int s)
		{
			int invs = 1 / s;
			return ((*this) *= invs);
		}

		int3& operator += (int s)
		{
			x += s;
			y += s;
			z += s;
			return *this;
		}

		int3& operator -= (int s)
		{
			x -= s;
			y -= s;
			z -= s;
			return *this;
		}

		/////////////////////////////////////////////
		//	其他运算
		/////////////////////////////////////////////
		int length_sqr() const
		{
			return x*x + y*y + z*z;
		}

		float length() const
		{
			return std::sqrt(static_cast<float>(length_sqr()));
		}

		void normalize()
		{
			int len = static_cast<int>(length());
			(*this) /= len;
		}
	private:
		bool operator == (const int3& rhs);
		bool operator != (const int3& rhs);
	};

	struct int4
	{
#ifdef EFLIB_MSVC
#pragma warning(push)
#pragma warning(disable : 4201)
#endif
		union{
			struct {
				int x, y, z, w;
			};
			struct {
				int s, t, r, q;
			};
		};
#ifdef EFLIB_MSVC
#pragma warning(pop)
#endif

		/////////////////////////////////////
		//		标准迭代器
		/////////////////////////////////////
		typedef const int*  const_iterator;
		typedef int* iterator;

		const_iterator begin() const
		{
			return (const_iterator)this;
		}

		const_iterator end() const
		{
			return ((const_iterator)this) + 4;
		}

		iterator begin()
		{
			return (iterator)this;
		}

		iterator end()
		{
			return ((iterator)this) + 4;
		}

		///////////////////////////////////////
		//	类型转换
		///////////////////////////////////////
		int* ptr()
		{
			return (int*)this;
		}

		const int* ptr() const
		{
			return (const int*)this;
		}
		
		const int& operator [](size_t i) const
		{
			custom_assert(i < 3, "");
			return ((int*)(this))[i];
		}

		int& operator [](size_t i)
		{
			custom_assert(i < 3, "");
			return ((int*)(this))[i];
		}
		///////////////////////////////////////////////
		//	构造函数
		///////////////////////////////////////////////
		int4(){}

		explicit int4(int x, int y = 0, int z = 0, int w = 0):x(x), y(y), z(z), w(w){}
		
		int4(const int4& v) : x(v.x), y(v.y), z(v.z), w(v.w){}
		
		int4(const int3& v, int w): x(v.x), y(v.y), z(v.z), w(w){}

		int4(const int* fv, int sizefv)
		{
			int clamped_size = std::max(sizefv, 4);
			for(int i = 0; i < clamped_size; ++i) 
				(*this)[i] = fv[i];
		}

		//////////////////////////////////////////////////
		//	四则运算。乘除为逐元素运算
		/////////////////////////////////////////////////
		int4 operator - () const
		{
			return int4(-x, -y, -z, -w);
		}

		int4& operator += (const int4& v)
		{
			x += v.x;
			y += v.y;
			z += v.z;
			w += v.w;
			return *this;
		}

		int4& operator -= (const int4& v)
		{
			x -= v.x;
			y -= v.y;
			z -= v.z;
			w -= v.w;
			return *this;
		}

		int4& operator *= (const int4& v)
		{
			x *= v.x;
			y *= v.y;
			z *= v.z;
			w *= v.w;
			return *this;
		}

		int4& operator /= (const int4& v)
		{
			x /= v.x;
			y /= v.y;
			z /= v.z;
			w /= v.w;
			return *this;
		}

		int4& operator *= (int s)
		{
			x *= s;
			y *= s;
			z *= s;
			w *= s;
			return *this;
		}

		int4& operator /= (int s)
		{
			int invs = 1 / s;
			return ((*this) *= invs);
		}

		int4& operator += (int s)
		{
			x += s;
			y += s;
			z += s;
			w += s;
			return *this;
		}

		int4& operator -= (int s)
		{
			x -= s;
			y -= s;
			z -= s;
			w -= s;
			return *this;
		}

		/////////////////////////////////////////////
		//	其他运算
		/////////////////////////////////////////////
		int length_sqr() const
		{
			return x*x + y*y + z*z + w*w;
		}

		float length() const
		{
			return std::sqrt(static_cast<float>(length_sqr()));
		}

		void normalize()
		{
			int len = static_cast<int>(length());
			(*this) /= len;
		}

		//static function
		static int4 zero(){
			return int4(0, 0, 0, 0);
		}

		static int4 gen_coord(int x, int y, int z){
			return int4(x, y, z, 1);
		}

		static int4 gen_vector(int x, int y, int z){
			return int4(x, y, z, 0);
		}

	private:
		bool operator == (const int4& rhs);
		bool operator != (const int4& rhs);
	};

	//non-member non-friend

	//int2
	inline int2 operator + (const int2& lhs, const int2& rhs){
		return int2(lhs.x+rhs.x, lhs.y+rhs.y);
	}

	inline int2 operator - (const int2& lhs, const int2& rhs){
		return int2(lhs.x-rhs.x, lhs.y-rhs.y);
	}

	inline int2 operator + (const int2& lhs, int s){
		return int2(lhs.x+s, lhs.y+s);
	}

	inline int2 operator + (int s, const int2& lhs){
		return lhs + s;
	}

	inline int2 operator - (const int2& lhs, int s){
		return int2(lhs.x-s, lhs.y-s);
	}

	inline int2 operator - (int s, const int2& lhs){
		return lhs - s;
	}

	inline int2 operator * (const int2& lhs, int s){
		return int2(lhs.x*s, lhs.y*s);
	}

	inline int2 operator * (int s, const int2& lhs){
		return lhs * s;
	}

	inline int2 operator * (const int2& lhs, const int2& rhs){
		return int2(lhs.x * rhs.x, lhs.y * rhs.y);
	}

	inline int2 operator / (const int2& lhs, int s){
		int invs = 1 / s;
		return lhs * invs;
	}

	inline int2 operator / (const int2& lhs, const int2& rhs){
		return int2(lhs.x / rhs.x, lhs.y / rhs.y);
	}

	//int3
	inline int3 operator + (const int3& lhs, const int3& rhs){
		return int3(lhs.x+rhs.x, lhs.y+rhs.y, lhs.z+rhs.z);
	}

	inline int3 operator - (const int3& lhs, const int3& rhs){
		return int3(lhs.x-rhs.x, lhs.y-rhs.y, lhs.z-rhs.z);
	}

	inline int3 operator + (const int3& lhs, int s){
		return int3(lhs.x+s, lhs.y+s, lhs.z+s);
	}

	inline int3 operator + (int s, const int3& lhs){
		return lhs + s;
	}

	inline int3 operator - (const int3& lhs, int s){
		return int3(lhs.x-s, lhs.y-s, lhs.z-s);
	}

	inline int3 operator - (int s, const int3& lhs){
		return lhs - s;
	}

	inline int3 operator * (const int3& lhs, int s){
		return int3(lhs.x*s, lhs.y*s, lhs.z*s);
	}

	inline int3 operator * (int s, const int3& lhs){
		return lhs * s;
	}

	inline int3 operator * (const int3& lhs, const int3& rhs){
		return int3(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z);
	}

	inline int3 operator / (const int3& lhs, int s){
		int invs = 1 / s;
		return lhs * invs;
	}

	inline int3 operator / (const int3& lhs, const int3& rhs){
		return int3(lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z);
	}

	//int4
	inline int4 operator + (const int4& lhs, const int4& rhs){
		return int4(lhs.x+rhs.x, lhs.y+rhs.y, lhs.z+rhs.z, lhs.w+rhs.w);
	}

	inline int4 operator - (const int4& lhs, const int4& rhs){
		return int4(lhs.x-rhs.x, lhs.y-rhs.y, lhs.z-rhs.z, lhs.w-rhs.w);
	}

	inline int4 operator + (const int4& lhs, int s){
		return int4(lhs.x+s, lhs.y+s, lhs.z+s, lhs.w+s);
	}

	inline int4 operator + (int s, const int4& lhs){
		return lhs + s;
	}

	inline int4 operator - (const int4& lhs, int s){
		return int4(lhs.x-s, lhs.y-s, lhs.z-s, lhs.w-s);
	}

	inline int4 operator - (int s, const int4& lhs){
		return lhs - s;
	}

	inline int4 operator * (const int4& lhs, int s){
		return int4(lhs.x*s, lhs.y*s, lhs.z*s, lhs.w*s);
	}

	inline int4 operator * (int s, const int4& lhs){
		return lhs * s;
	}

	inline int4 operator * (const int4& lhs, const int4& rhs){
		return int4(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z, lhs.w * rhs.w);
	}

	inline int4 operator / (const int4& lhs, int s){
		int invs = 1 / s;
		return lhs * invs;
	}

	inline int4 operator / (const int4& lhs, const int4& rhs){
		return int4(lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z, lhs.w / rhs.w);
	}
} //namespace
#endif
