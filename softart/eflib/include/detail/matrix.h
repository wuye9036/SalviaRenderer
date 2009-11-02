#ifndef EFLIB_MATRIX_H
#define EFLIB_MATRIX_H

#include "vector.h"

namespace efl
{
	class mat44
	{
	public:
#ifdef EFLIB_MSVC
#pragma warning(push)
#pragma warning(disable : 4201)
#endif
		union
		{
			struct 
			{
				vec4 v0, v1, v2, v3;
			};
			float f[4][4];
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
			custom_assert(i < 4, "");
			return vec4(f[0][i], f[1][i], f[2][i], f[3][i]);
		}

		void set_column(size_t i, float _1, float _2, float _3, float _4)
		{
			custom_assert(i < 4, "");
			f[0][i] = _1;
			f[1][i] = _2;
			f[2][i] = _3;
			f[3][i] = _4;
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
			: v0(_11, _12, _13, _14),
			v1(_21, _22, _23, _24),
			v2(_31, _32, _33, _34),
			v3(_41, _42, _43, _44)
		{
		}

		mat44(const vec4& v0, const vec4& v1, const vec4& v2, const vec4& v3)
			: v0(v0), v1(v1), v2(v2), v3(v3)
		{
		}

		mat44(const float* f){
			memcpy((void*)this, (const void*)f, sizeof(mat44));
		}

		/******************************************
		*  赋值与拷贝构造
		*
		*****************************************/
		mat44(const mat44& m) : v0(m.v0), v1(m.v1), v2(m.v2), v3(m.v3){}

		mat44& operator = (const mat44& m)
		{
			v0 = m.v0;
			v1 = m.v1;
			v2 = m.v2;
			v3 = m.v3;
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
				v0+m.v0, v1+m.v1, v2+m.v2, v3+m.v3
				);
		}

		mat44 operator - (const mat44& m)
		{
			return mat44(
				v0-m.v0, v1-m.v1, v2-m.v2, v3-m.v3
				);
		}

		mat44& operator += (const mat44& m)
		{
			v0 += m.v0;
			v1 += m.v1;
			v2 += m.v2;
			v3 += m.v3;
			return *this;
		}

		mat44& operator -= (const mat44& m)
		{
			v0 -= m.v0;
			v1 -= m.v1;
			v2 -= m.v2;
			v3 -= m.v3;
			return *this;
		}

		/********数vs矩阵，只提供乘除*********/
		mat44 operator * (float s)
		{
			return mat44(v0*s, v1*s, v2*s, v3*s);
		}

		mat44 operator / (float s)
		{
			float invs = 1.0f/s;
			return (*this) * invs;
		}

		mat44& operator *= (float s)
		{
			v0*=s; v1*=s; v2*=s; v3*=s;
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