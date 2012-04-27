#ifndef EFLIB_MATH_VECTOR_GENERIC_H
#define EFLIB_MATH_VECTOR_GENERIC_H

#include <eflib/include/platform/config.h>

#include <eflib/include/math/swizzle.h>
#include <eflib/include/math/write_mask.h>
#include <eflib/include/diagnostics/assert.h>

namespace eflib
{
	template <typename ScalarT, int Size> struct vector_;

	template <typename ScalarT, int Size>
	struct vector_data
	{
		ScalarT data_[Size];

		vector_<ScalarT, Size>* derived_this()
		{
			return static_cast<vector_<ScalarT,Size>*>(this);
		}

		vector_<ScalarT, Size> const* derived_this() const
		{
			return static_cast<vector_<ScalarT,Size> const*>(this);
		}

		template <typename IndexT>
		ScalarT const& operator [](IndexT index) const
		{
			EFLIB_ASSERT(index<Size, "Out of bound.");
			return data_[index];
		}

		template <typename IndexT>
		ScalarT& operator [](IndexT index)
		{
			EFLIB_ASSERT(index<Size, "Out of bound.");
			return data_[index];
		}

		typedef ScalarT				element_type;
		static unsigned int const	vector_size = Size;

		//
		vector_<ScalarT,Size>& set(vector_<ScalarT,Size> const& v)
		{
			if( &v == this ) return;
			for( int index = 0; index < Size; ++index )
			{
				data_[index] = v.data_[index];
			}
			return *this;
		}

		// Arithmetics
		vector_<ScalarT,Size> operator -() const
		{
			vector_<ScalarT,Size> ret;
			for( int index = 0; index < Size; ++index )
			{
				ret.data_[index] = -data_[index];
			}
			return ret;
		}

		vector_<ScalarT,Size>& operator *= (vector_<ScalarT,Size> const& v)
		{
			for( int index = 0; index < Size; ++index )
			{
				data_[index] *= v[index];
			}
			return *derived_this();
		}

		vector_<ScalarT,Size>& operator *= (float s)
		{
			for( int index = 0; index < Size; ++index )
			{
				data_[index] *= s;
			}
			return *derived_this();
		}

		vector_<ScalarT,Size>& operator /= (float s)
		{
			float invs = 1 / s;
			(*this) *= invs;
			return *derived_this();
		}

		vector_<ScalarT,Size>& operator += (vector_<ScalarT,Size> const& v)
		{
			for( int index = 0; index < Size; ++index )
			{
				data_[index] += v[index];
			}
			return *derived_this();
		}

		vector_<ScalarT,Size>& operator += (float s)
		{
			for( int index = 0; index < Size; ++index )
			{
				data_[index] += s;
			}
			return *this;
		}

		vector_<ScalarT,Size>& operator -= (vector_<ScalarT,Size> const& v)
		{
			for( int index = 0; index < Size; ++index )
			{
				data_[index] -= v[index];
			}
			return *derived_this();
		}

		vector_<ScalarT,Size>& operator -= (float s)
		{
			for( int index = 0; index < Size; ++index )
			{
				data_[index] -= s;
			}
			return *this;
		}

		// Functions
		ScalarT length_sqr() const
		{
			ScalarT total = ScalarT(0);
			for( int i = 0; i < Size; ++i )
			{
				total += data_[i] * data_[i];
			}
			return total;
		}

		ScalarT length() const
		{
			return std::sqrt(length_sqr());
		}

		void normalize()
		{
			ScalarT len = length();
			(*this) /= len;
		}
	};

	template <typename ScalarT, int i> struct vector_swizzle;

	template <typename ScalarT>
	struct vector_swizzle<ScalarT, 0>
	{
	};

	template <typename ScalarT>
	struct vector_swizzle<ScalarT, 1>
	{
		ScalarT	const&		x() const;
		vector_<ScalarT, 2> xx() const;
		vector_<ScalarT, 3> xxx() const;
		vector_<ScalarT, 4> xxx() const;
	};

	template <typename ScalarT>
	struct vector_swizzle<ScalarT, 2>
	{
		SWIZZLE_DECL_FOR_VEC2();
		WRITE_MASK_FOR_VEC2();
	};

	template <typename ScalarT>
	struct vector_swizzle<ScalarT, 3>
	{
		SWIZZLE_DECL_FOR_VEC3();
		WRITE_MASK_FOR_VEC3();
	};

	template <typename ScalarT>
	struct vector_swizzle<ScalarT, 4>
	{
		SWIZZLE_DECL_FOR_VEC4();
		WRITE_MASK_FOR_VEC4();
	};

	template <typename ScalarT, int Size> struct vector_{};

	template <typename ScalarT>
	struct vector_<ScalarT,1>: public vector_swizzle<ScalarT, 1>, public vector_data<ScalarT, 1>
	{
		vector_<ScalarT,1>() {}
		explicit vector_<ScalarT,1>( ScalarT v ){ data_[0] = v; }
	};

	template <typename ScalarT>
	struct vector_<ScalarT,2>: public vector_swizzle<ScalarT,2>, public vector_data<ScalarT,2>
	{
		vector_<ScalarT,2>() {}
		explicit vector_<ScalarT,2>( ScalarT v0, ScalarT v1 )
		{
			data_[0] = v0;
			data_[1] = v1;
		}
	};

	template <typename ScalarT>
	struct vector_<ScalarT,3>: public vector_swizzle<ScalarT,3>, public vector_data<ScalarT,3>
	{
		vector_<ScalarT,3>() {}
		explicit vector_<ScalarT,3>( ScalarT v0, ScalarT v1, ScalarT v2 )
		{
			data_[0] = v0;
			data_[1] = v1;
			data_[2] = v2;
		}
	};

	template <typename ScalarT>
	struct vector_<ScalarT,4>: public vector_swizzle<ScalarT,4>, public vector_data<ScalarT,4>
	{
		vector_<ScalarT,4>() {}
		explicit vector_<ScalarT,4>( ScalarT v0, ScalarT v1=ScalarT(0), ScalarT v2=ScalarT(0), ScalarT v3=ScalarT(0) )
		{
			data_[0] = v0;
			data_[1] = v1;
			data_[2] = v2;
			data_[3] = v3;
		}
		explicit vector_<ScalarT,4>( vector_<ScalarT,3> const& v, ScalarT s=ScalarT(0) )
		{
			data_[0] = v[0];
			data_[1] = v[1];
			data_[2] = v[2];
			data_[3] = s;
		}

		void normalize3()
		{
			this->xyz().normalize();
		}

		void projection()
		{
			data_[0] /= data_[3];
			data_[1] /= data_[3];
			data_[2] /= data_[3];
			data_[3] = 1.0f;
		}

		// Special vectors
		static vector_<ScalarT,4> zero(){
			return vec4(ScalarT(0), ScalarT(0), ScalarT(0), ScalarT(0));
		}

		static vector_<ScalarT,4> gen_coord(ScalarT x, ScalarT y, ScalarT z){
			return vec4(x, y, z, 1.0f);
		}

		static vector_<ScalarT,4> gen_vector(ScalarT x, ScalarT y, ScalarT z){
			return vec4(x, y, z, 0.0f);
		}
	};

	SWIZZLE_IMPL_FOR_VEC2();
	SWIZZLE_IMPL_FOR_VEC3();
	SWIZZLE_IMPL_FOR_VEC4();

	template <typename ScalarT, int Size>
	inline vector_<ScalarT,Size> operator + (vector_<ScalarT,Size> const& lhs, vector_<ScalarT,Size> const& rhs)
	{
		vector_<ScalarT,Size> ret;
		for( int i = 0; i < Size; ++i ){ ret[i] = lhs[i] + rhs[i]; }
		return ret;
	}

	template <typename ScalarT, int Size>
	inline vector_<ScalarT,Size> operator - (vector_<ScalarT,Size> const& lhs, vector_<ScalarT,Size> const& rhs){
		vector_<ScalarT,Size> ret;
		for( int i = 0; i < Size; ++i ){ ret[i] = lhs[i] - rhs[i]; }
		return ret;
	}

	template <typename ScalarT, int Size>
	inline vector_<ScalarT,Size> operator + (vector_<ScalarT,Size> const& lhs, float s){
		vector_<ScalarT,Size> ret;
		for( int i = 0; i < Size; ++i ){ ret[i] = lhs[i] + s; }
		return ret;
	}

	template <typename ScalarT, int Size>
	inline vector_<ScalarT,Size> operator + (float s, vector_<ScalarT,Size> const& lhs){
		return lhs + s;
	}

	template <typename ScalarT, int Size>
	inline vector_<ScalarT,Size> operator - (vector_<ScalarT,Size> const& lhs, float s){
		vector_<ScalarT,Size> ret;
		for( int i = 0; i < Size; ++i ){ ret[i] = lhs[i] - s; }
		return ret;
	}

	template <typename ScalarT, int Size>
	inline vector_<ScalarT,Size> operator - (float s, vector_<ScalarT,Size> const& lhs){
		vector_<ScalarT,Size> ret;
		for( int i = 0; i < Size; ++i ){ ret[i] = s - lhs[i]; }
		return ret;
	}

	template <typename ScalarT, int Size>
	inline vector_<ScalarT,Size> operator * (vector_<ScalarT,Size> const& lhs, vector_<ScalarT,Size> const& rhs)
	{
		vector_<ScalarT,Size> ret;
		for( int i = 0; i < Size; ++i ){ ret[i] = lhs[i] * rhs[i]; }
		return ret;
	}

	template <typename ScalarT, int Size>
	inline vector_<ScalarT,Size> operator / (vector_<ScalarT,Size> const& lhs, vector_<ScalarT,Size> const& rhs){
		vector_<ScalarT,Size> ret;
		for( int i = 0; i < Size; ++i ){ ret[i] = lhs[i] / rhs[i]; }
		return ret;
	}

	template <typename ScalarT, int Size>
	inline vector_<ScalarT,Size> operator * (vector_<ScalarT,Size> const& lhs, float s){
		vector_<ScalarT,Size> ret;
		for( int i = 0; i < Size; ++i ){ ret[i] = lhs[i] * s; }
		return ret;
	}

	template <typename ScalarT, int Size>
	inline vector_<ScalarT,Size> operator * (float s, vector_<ScalarT,Size> const& lhs){
		return lhs * s;
	}

	template <typename ScalarT, int Size>
	inline vector_<ScalarT,Size> operator / (vector_<ScalarT,Size> const& lhs, float s){
		vector_<ScalarT,Size> ret;
		for( int i = 0; i < Size; ++i ){ ret[i] = lhs[i] / s; }
		return ret;
	}

	template <typename ScalarT, int Size>
	inline vector_<ScalarT,Size> operator / (float s, vector_<ScalarT,Size> const& lhs){
		vector_<ScalarT,Size> ret;
		for( int i = 0; i < Size; ++i ){ ret[i] = s / lhs[i]; }
		return ret;
	}
}

#endif