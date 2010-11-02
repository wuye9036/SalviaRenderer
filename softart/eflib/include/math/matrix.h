#ifndef EFLIB_MATH_MATRIX_H
#define EFLIB_MATH_MATRIX_H

#include "vector.h"
#include <memory.h>

namespace eflib
{
	class mat44
	{
	public:
#ifdef EFLIB_MSVC
#pragma warning(push)
#pragma warning(disable : 4201)
#endif
		float f[4][4];
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
			return ((const_iterator)this) + 16;
		}

		iterator begin()
		{
			return (iterator)this;
		}

		iterator end()
		{
			return ((iterator)this) + 16;
		}

		/******************************************
		*  数据获取函数
		******************************************/
		vec4 get_column(size_t i) const
		{
			EFLIB_ASSERT(i < 4, "");
#ifdef EFLIB_MSVC
#pragma warning(push)
#pragma warning(disable: 6385 6386)
#endif
			return vec4(f[0][i], f[1][i], f[2][i], f[3][i]);
#ifdef EFLIB_MSVC
#pragma warning(pop)
#endif
		}

		void set_column(size_t i, float _1, float _2, float _3, float _4)
		{
			EFLIB_ASSERT(i < 4, "");
#ifdef EFLIB_MSVC
#pragma warning(push)
#pragma warning(disable: 6385 6386)
#endif
			f[0][i] = _1;
			f[1][i] = _2;
			f[2][i] = _3;
			f[3][i] = _4;
#ifdef EFLIB_MSVC
#pragma warning(pop)
#endif
		}

		void set_column(size_t i, const vec4& v)
		{
			EFLIB_ASSERT(i < 4, "");
#ifdef EFLIB_MSVC
#pragma warning(push)
#pragma warning(disable: 6385 6386)
#endif
			f[0][i] = v.x;
			f[1][i] = v.y;
			f[2][i] = v.z;
			f[3][i] = v.w;
#ifdef EFLIB_MSVC
#pragma warning(pop)
#endif
		}

		vec4 get_row(size_t i) const
		{
			EFLIB_ASSERT(i < 4, "");
#ifdef EFLIB_MSVC
#pragma warning(push)
#pragma warning(disable: 6385 6386)
#endif
			return vec4(f[i][0], f[i][1], f[i][2], f[i][3]);
#ifdef EFLIB_MSVC
#pragma warning(pop)
#endif
		}

		void set_row(size_t i, float _1, float _2, float _3, float _4)
		{
			EFLIB_ASSERT(i < 4, "");
#ifdef EFLIB_MSVC
#pragma warning(push)
#pragma warning(disable: 6385 6386)
#endif
			f[i][0] = _1;
			f[i][1] = _2;
			f[i][2] = _3;
			f[i][3] = _4;
#ifdef EFLIB_MSVC
#pragma warning(pop)
#endif
		}

		void set_row(size_t i, const vec4& v)
		{
			EFLIB_ASSERT(i < 4, "");
#ifdef EFLIB_MSVC
#pragma warning(push)
#pragma warning(disable: 6385 6386)
#endif
			f[i][0] = v.x;
			f[i][1] = v.y;
			f[i][2] = v.z;
			f[i][3] = v.w;
#ifdef EFLIB_MSVC
#pragma warning(pop)
#endif
		}

		//This route is from KlayGE (Author by GMM)
		float det() const
		{
			float _3142_3241(this->f[2][0] * this->f[3][1] - this->f[2][1] * this->f[3][0]);
			float _3143_3341(this->f[2][0] * this->f[3][2] - this->f[2][2] * this->f[3][0]);
			float _3144_3441(this->f[2][0] * this->f[3][3] - this->f[2][3] * this->f[3][0]);
			float _3243_3342(this->f[2][1] * this->f[3][2] - this->f[2][2] * this->f[3][1]);
			float _3244_3442(this->f[2][1] * this->f[3][3] - this->f[2][3] * this->f[3][1]);
			float _3344_3443(this->f[2][2] * this->f[3][3] - this->f[2][3] * this->f[3][2]);

			return
				this->f[0][0] * (this->f[1][1] * _3344_3443 - this->f[1][2] * _3244_3442 + this->f[1][3] * _3243_3342)
				- this->f[0][1] * (this->f[1][0] * _3344_3443 - this->f[1][2] * _3144_3441 + this->f[1][3] * _3143_3341)
				+ this->f[0][2] * (this->f[1][0] * _3244_3442 - this->f[1][1] * _3144_3441 + this->f[1][3] * _3142_3241)
				- this->f[0][3] * (this->f[1][0] * _3243_3342 - this->f[1][1] * _3143_3341 + this->f[1][2] * _3142_3241);
		}
		/******************************************
		*  构造函数
		*
		*****************************************/
		mat44(){}

		mat44(
			float _11, float _12, float _13, float _14,
			float _21, float _22, float _23, float _24,
			float _31, float _32, float _33, float _34,
			float _41, float _42, float _43, float _44
			)
		{
			f[0][0] = _11;
			f[0][1] = _12;
			f[0][2] = _13;
			f[0][3] = _14;

			f[1][0] = _21;
			f[1][1] = _22;
			f[1][2] = _23;
			f[1][3] = _24;

			f[2][0] = _31;
			f[2][1] = _32;
			f[2][2] = _33;
			f[2][3] = _34;

			f[3][0] = _41;
			f[3][1] = _42;
			f[3][2] = _43;
			f[3][3] = _44;
		}

		mat44(const vec4& v0, const vec4& v1, const vec4& v2, const vec4& v3)
		{
			f[0][0] = v0.x;
			f[0][1] = v0.y;
			f[0][2] = v0.z;
			f[0][3] = v0.w;

			f[1][0] = v1.x;
			f[1][1] = v1.y;
			f[1][2] = v1.z;
			f[1][3] = v1.w;

			f[2][0] = v2.x;
			f[2][1] = v2.y;
			f[2][2] = v2.z;
			f[2][3] = v2.w;

			f[3][0] = v3.x;
			f[3][1] = v3.y;
			f[3][2] = v3.z;
			f[3][3] = v3.w;
		}

		mat44(const float* _f){
			memcpy(f, _f, sizeof(mat44));
		}

		/******************************************
		*  赋值与拷贝构造
		*
		*****************************************/
		mat44(const mat44& m){
			memcpy(f, m.f, sizeof(mat44));
		}

		mat44& operator = (const mat44& m)
		{
			memcpy(f, m.f, sizeof(mat44));
			return *this;
		}

		template<int lb0, int lb1, int ub0, int ub1>
		void assign_submat(const mat44& m)
		{
			for(int i = lb0; i < ub0; ++i)
			{
				for(int j = lb1; j < ub1; ++j)
				{
					f[i][j] = m.f[i][j];
				}
			}
		}

		/******************************************
		*	 四则运算
		*
		*****************************************/

		/********逐元素操作 矩阵vs矩阵 只提供加减*********/
		mat44 operator + (const mat44& m)
		{
			return mat44(
				f[0][0] + m.f[0][0],
				f[0][1] + m.f[0][1],
				f[0][2] + m.f[0][2],
				f[0][3] + m.f[0][3],

				f[1][0] + m.f[1][0],
				f[1][1] + m.f[1][1],
				f[1][2] + m.f[1][2],
				f[1][3] + m.f[1][3],

				f[2][0] + m.f[2][0],
				f[2][1] + m.f[2][1],
				f[2][2] + m.f[2][2],
				f[2][3] + m.f[2][3],

				f[3][0] + m.f[3][0],
				f[3][1] + m.f[3][1],
				f[3][2] + m.f[3][2],
				f[3][3] + m.f[3][3]
				);
		}

		mat44 operator - (const mat44& m)
		{
			return mat44(
				f[0][0] - m.f[0][0],
				f[0][1] - m.f[0][1],
				f[0][2] - m.f[0][2],
				f[0][3] - m.f[0][3],

				f[1][0] - m.f[1][0],
				f[1][1] - m.f[1][1],
				f[1][2] - m.f[1][2],
				f[1][3] - m.f[1][3],

				f[2][0] - m.f[2][0],
				f[2][1] - m.f[2][1],
				f[2][2] - m.f[2][2],
				f[2][3] - m.f[2][3],

				f[3][0] - m.f[3][0],
				f[3][1] - m.f[3][1],
				f[3][2] - m.f[3][2],
				f[3][3] - m.f[3][3]
				);
		}

		mat44& operator += (const mat44& m)
		{
			f[0][0] += m.f[0][0];
			f[0][1] += m.f[0][1];
			f[0][2] += m.f[0][2];
			f[0][3] += m.f[0][3];

			f[1][0] += m.f[1][0];
			f[1][1] += m.f[1][1];
			f[1][2] += m.f[1][2];
			f[1][3] += m.f[1][3];

			f[2][0] += m.f[2][0];
			f[2][1] += m.f[2][1];
			f[2][2] += m.f[2][2];
			f[2][3] += m.f[2][3];

			f[3][0] += m.f[3][0];
			f[3][1] += m.f[3][1];
			f[3][2] += m.f[3][2];
			f[3][3] += m.f[3][3];

			return *this;
		}

		mat44& operator -= (const mat44& m)
		{
			f[0][0] -= m.f[0][0];
			f[0][1] -= m.f[0][1];
			f[0][2] -= m.f[0][2];
			f[0][3] -= m.f[0][3];

			f[1][0] -= m.f[1][0];
			f[1][1] -= m.f[1][1];
			f[1][2] -= m.f[1][2];
			f[1][3] -= m.f[1][3];

			f[2][0] -= m.f[2][0];
			f[2][1] -= m.f[2][1];
			f[2][2] -= m.f[2][2];
			f[2][3] -= m.f[2][3];

			f[3][0] -= m.f[3][0];
			f[3][1] -= m.f[3][1];
			f[3][2] -= m.f[3][2];
			f[3][3] -= m.f[3][3];

			return *this;
		}

		/********数vs矩阵，只提供乘除*********/
		mat44 operator * (float s)
		{
			return mat44(
				f[0][0] * s,
				f[0][1] * s,
				f[0][2] * s,
				f[0][3] * s,

				f[1][0] * s,
				f[1][1] * s,
				f[1][2] * s,
				f[1][3] * s,

				f[2][0] * s,
				f[2][1] * s,
				f[2][2] * s,
				f[2][3] * s,

				f[3][0] * s,
				f[3][1] * s,
				f[3][2] * s,
				f[3][3] * s
			);
		}

		mat44 operator / (float s)
		{
			float invs = 1.0f/s;
			return (*this) * invs;
		}

		mat44& operator *= (float s)
		{
			f[0][0] *= s;
			f[0][1] *= s;
			f[0][2] *= s;
			f[0][3] *= s;

			f[1][0] *= s;
			f[1][1] *= s;
			f[1][2] *= s;
			f[1][3] *= s;

			f[2][0] *= s;
			f[2][1] *= s;
			f[2][2] *= s;
			f[2][3] *= s;

			f[3][0] *= s;
			f[3][1] *= s;
			f[3][2] *= s;
			f[3][3] *= s;

			return *this;
		}

		mat44& operator /= (float s)
		{
			float invs = 1.0f/s;
			return ((*this)*=invs);
		}
		/******************************************
		*  特殊矩阵
		*
		*****************************************/
		static mat44 zero()
		{
			return diag(0.0f, 0.0f, 0.0f, 0.0f);
		}

		static mat44 diag(float _11, float _22, float _33, float _44)
		{
			return mat44(
				_11, 0.0f, 0.0f, 0.0f,
				0.0f, _22, 0.0f, 0.0f,
				0.0f, 0.0f, _33, 0.0f,
				0.0f, 0.0f, 0.0f, _44
				);
		}

		static mat44 identity()
		{
			return diag(1.0f, 1.0f, 1.0f, 1.0f);
		}
	};
}
#endif
