#ifndef EFLIB_MATH_MATRIX_GENERIC_H
#define EFLIB_MATH_MATRIX_GENERIC_H

#include <eflib/include/platform/config.h>

namespace eflib
{
	template <typename ScalarT, int ColumnSize, int RowSize>
	struct matrix_;

	template <typename ScalarT, int ColumnSize, int RowSize>
	struct matrix_data;

	template <typename ScalarT, int Size>
	struct vector_;

	template <typename ScalarT, int ColumnSize, int RowSize>
	struct det_ { };

	template <typename ScalarT>
	struct det_<ScalarT, 4, 4>
	{
		ScalarT det() const
		{
			matrix_<ScalarT,4,4> const* derived_this = static_cast<matrix_<ScalarT,4,4> const*>(this);

			ScalarT _3142_3241(derived_this->data_[2][0] * derived_this->data_[3][1] - derived_this->data_[2][1] * derived_this->data_[3][0]);
			ScalarT _3143_3341(derived_this->data_[2][0] * derived_this->data_[3][2] - derived_this->data_[2][2] * derived_this->data_[3][0]);
			ScalarT _3144_3441(derived_this->data_[2][0] * derived_this->data_[3][3] - derived_this->data_[2][3] * derived_this->data_[3][0]);
			ScalarT _3243_3342(derived_this->data_[2][1] * derived_this->data_[3][2] - derived_this->data_[2][2] * derived_this->data_[3][1]);
			ScalarT _3244_3442(derived_this->data_[2][1] * derived_this->data_[3][3] - derived_this->data_[2][3] * derived_this->data_[3][1]);
			ScalarT _3344_3443(derived_this->data_[2][2] * derived_this->data_[3][3] - derived_this->data_[2][3] * derived_this->data_[3][2]);

			return
				  derived_this->data_[0][0] * (derived_this->data_[1][1] * _3344_3443 - derived_this->data_[1][2] * _3244_3442 + derived_this->data_[1][3] * _3243_3342)
				- derived_this->data_[0][1] * (derived_this->data_[1][0] * _3344_3443 - derived_this->data_[1][2] * _3144_3441 + derived_this->data_[1][3] * _3143_3341)
				+ derived_this->data_[0][2] * (derived_this->data_[1][0] * _3244_3442 - derived_this->data_[1][1] * _3144_3441 + derived_this->data_[1][3] * _3142_3241)
				- derived_this->data_[0][3] * (derived_this->data_[1][0] * _3243_3342 - derived_this->data_[1][1] * _3143_3341 + derived_this->data_[1][2] * _3142_3241);
		}
	};

	template <typename ScalarT, int ColumnSize, int RowSize>
	struct matrix_data
	{
		ScalarT data_[RowSize][ColumnSize];

		template <typename IndexT>
		vector_<ScalarT, RowSize> get_column(IndexT i) const
		{
			vector_<ScalarT, RowSize> ret;
			for( int i_row = 0; i_row < RowSize; ++i_row )
			{
				ret[i_row] = data_[i_row][i];
			}
			return ret;
		}

		template <typename IndexT>
		void set_column( IndexT i, vector_<ScalarT, RowSize> const& v )
		{
			for( int i_row = 0; i_row < RowSize; ++i_row )
			{
				data_[i_row][i] = v[i_row];
			}
		}

		template <typename IndexT>
		vector_<ScalarT, ColumnSize> const& get_row(IndexT i) const
		{
			return *reinterpret_cast<vector_<ScalarT,ColumnSize> const*>(&(data_[i][0]));
		}

		template <typename IndexT>
		void set_row( IndexT i, vector_<ScalarT, ColumnSize> const& v )
		{
			for( int i_col = 0; i_col < ColumnSize; ++i_col )
			{
				data_[i][i_col] = v[i_col];
			}
		}

		typedef float*			iterator;
		typedef float const*	const_iterator;

		iterator		begin() { return &data_[0][0]; }
		const_iterator	begin() const { return &data_[0][0]; }

		iterator		end() { return begin() + RowSize*ColumnSize; }
		const_iterator	end() const { return begin() + RowSize*ColumnSize; }
	};

	template <typename ScalarT, int ColumnSize, int RowSize, bool IsSquare>
	struct matrix_operators: public det_<ScalarT, ColumnSize, RowSize>
	{
		static matrix_<ScalarT,ColumnSize,RowSize> zero()
		{
			matrix_<ScalarT,ColumnSize,RowSize> ret;
			for( size_t i = 0; i < ColumnSize*RowSize; ++i )
			{
				(&ret.data_[0][0])[i] = ScalarT(0);
			}
			return ret;
		}
	};

	template <typename ScalarT, int ColumnSize, int RowSize>
	struct matrix_operators<ScalarT,ColumnSize,RowSize,true>: public matrix_operators<ScalarT,ColumnSize,RowSize,false>
	{
		static matrix_<ScalarT,ColumnSize,RowSize> diag( ScalarT d0, ScalarT d1 = ScalarT(0), ScalarT d2 = ScalarT(0), ScalarT d3 = ScalarT(0) )
		{
			ScalarT diags[4] = {d0, d1, d2, d3}; 
			return diag(diags);
		}

		static matrix_<ScalarT,ColumnSize,RowSize> diag( ScalarT const* v )
		{
			matrix_<ScalarT,ColumnSize,RowSize> ret = zero();
			for( size_t i = 0; i < ColumnSize; ++i )
			{
				ret.data_[i][i] = v[i];
			}
			return ret;
		}

		static matrix_<ScalarT,ColumnSize,RowSize> identity()
		{
			return diag( ScalarT(1), ScalarT(1), ScalarT(1), ScalarT(1) );
		}
	};

	template <typename ScalarT, int ColumnSize, int RowSize>
	struct matrix_: matrix_operators<ScalarT, ColumnSize, RowSize, ColumnSize==RowSize>, matrix_data<ScalarT, ColumnSize, RowSize>
	{
		matrix_<ScalarT,ColumnSize,RowSize>(){}
		matrix_<ScalarT,ColumnSize,RowSize>( matrix_<ScalarT,ColumnSize,RowSize> const& v )
		{
			for( size_t i = 0; i < RowSize; ++i )
			{
				for( size_t j = 0; j < ColumnSize; ++j )
				data_[i][j] = v.data_[i][j];
			}
		}
	};

	template <typename ScalarT>
	struct matrix_<ScalarT,4,4>: matrix_operators<ScalarT,4,4,true>, matrix_data<ScalarT,4,4>
	{
		matrix_<ScalarT,4,4>(){}
		matrix_<ScalarT,4,4>( matrix_<ScalarT,4,4> const& v )
		{
			for( size_t i = 0; i < 4; ++i )
			{
				for( size_t j = 0; j < 4; ++j )
					data_[i][j] = v.data_[i][j];
			}
		}
		matrix_<ScalarT,4,4>(
			ScalarT _11, ScalarT _12, ScalarT _13, ScalarT _14,
			ScalarT _21, ScalarT _22, ScalarT _23, ScalarT _24,
			ScalarT _31, ScalarT _32, ScalarT _33, ScalarT _34,
			ScalarT _41, ScalarT _42, ScalarT _43, ScalarT _44
			)
		{
			data_[0][0] = _11;
			data_[0][1] = _12;
			data_[0][2] = _13;
			data_[0][3] = _14;
				
			data_[1][0] = _21;
			data_[1][1] = _22;
			data_[1][2] = _23;
			data_[1][3] = _24;
				
			data_[2][0] = _31;
			data_[2][1] = _32;
			data_[2][2] = _33;
			data_[2][3] = _34;
				
			data_[3][0] = _41;
			data_[3][1] = _42;
			data_[3][2] = _43;
			data_[3][3] = _44;
		}
	};

	template <typename ScalarT, int ColumnSize, int RowSize>
	inline matrix_<ScalarT, ColumnSize, RowSize> operator + (matrix_<ScalarT, ColumnSize, RowSize> const& lhs, matrix_<ScalarT, ColumnSize, RowSize> const& rhs)
	{
		matrix_<ScalarT, ColumnSize, RowSize> ret;
		for(int i = 0; i < RowSize; ++i)
		{
			for(int j = 0; j < ColumnSize; ++j)
			{
				ret.data_[i][j] = lhs.data_[i][j] + rhs.data_[i][j];
			}
		}
		return ret;
	}

	template <typename ScalarT, int ColumnSize, int RowSize>
	inline matrix_<ScalarT, ColumnSize, RowSize> operator - (matrix_<ScalarT, ColumnSize, RowSize> const& lhs, matrix_<ScalarT, ColumnSize, RowSize> const& rhs){
		matrix_<ScalarT, ColumnSize, RowSize> ret;
		for(int i = 0; i < RowSize; ++i)
		{
			for(int j = 0; j < ColumnSize; ++j)
			{
				ret.data_[i][j] = lhs.data_[i][j] - rhs.data_[i][j];
			}
		}
		return ret;
	}

	template <typename ScalarT, int ColumnSize, int RowSize>
	inline matrix_<ScalarT, ColumnSize, RowSize> operator + (matrix_<ScalarT, ColumnSize, RowSize> const& lhs, float s){
		matrix_<ScalarT, ColumnSize, RowSize> ret;
		for(int i = 0; i < RowSize; ++i)
		{
			for(int j = 0; j < ColumnSize; ++j)
			{
				ret.data_[i][j] = lhs.data_[i][j] + s;
			}
		}
		return ret;
	}

	template <typename ScalarT, int ColumnSize, int RowSize>
	inline matrix_<ScalarT, ColumnSize, RowSize> operator + (float s, matrix_<ScalarT, ColumnSize, RowSize> const& lhs){
		return lhs + s;
	}

	template <typename ScalarT, int ColumnSize, int RowSize>
	inline matrix_<ScalarT, ColumnSize, RowSize> operator - (matrix_<ScalarT, ColumnSize, RowSize> const& lhs, float s){
		matrix_<ScalarT, ColumnSize, RowSize> ret;
		for(int i = 0; i < RowSize; ++i)
		{
			for(int j = 0; j < ColumnSize; ++j)
			{
				ret.data_[i][j] = lhs.data_[i][j] - s;
			}
		}
		return ret;
	}

	template <typename ScalarT, int ColumnSize, int RowSize>
	inline matrix_<ScalarT, ColumnSize, RowSize> operator - (float s, matrix_<ScalarT, ColumnSize, RowSize> const& lhs){
		matrix_<ScalarT, ColumnSize, RowSize> ret;
		for(int i = 0; i < RowSize; ++i)
		{
			for(int j = 0; j < ColumnSize; ++j)
			{
				ret.data_[i][j] = s - lhs.data_[i][j];
			}
		}
		return ret;
	}

	template <typename ScalarT, int ColumnSize, int RowSize>
	inline matrix_<ScalarT, ColumnSize, RowSize> operator * (matrix_<ScalarT, ColumnSize, RowSize> const& lhs, matrix_<ScalarT, ColumnSize, RowSize> const& rhs)
	{
		matrix_<ScalarT, ColumnSize, RowSize> ret;
		for(int i = 0; i < RowSize; ++i)
		{
			for(int j = 0; j < ColumnSize; ++j)
			{
				ret.data_[i][j] = lhs.data_[i][j] * rhs.data_[i][j];
			}
		}
		return ret;
	}

	template <typename ScalarT, int ColumnSize, int RowSize>
	inline matrix_<ScalarT, ColumnSize, RowSize> operator / (matrix_<ScalarT, ColumnSize, RowSize> const& lhs, matrix_<ScalarT, ColumnSize, RowSize> const& rhs){
		matrix_<ScalarT, ColumnSize, RowSize> ret;
		for(int i = 0; i < RowSize; ++i)
		{
			for(int j = 0; j < ColumnSize; ++j)
			{
				ret.data_[i][j] = lhs.data_[i][j] / rhs.data_[i][j];
			}
		}
		return ret;
	}

	template <typename ScalarT, int ColumnSize, int RowSize>
	inline matrix_<ScalarT, ColumnSize, RowSize> operator * (matrix_<ScalarT, ColumnSize, RowSize> const& lhs, float s){
		matrix_<ScalarT, ColumnSize, RowSize> ret;
		for(int i = 0; i < RowSize; ++i)
		{
			for(int j = 0; j < ColumnSize; ++j)
			{
				ret.data_[i][j] = lhs.data_[i][j] * s;
			}
		}
		return ret;
	}

	template <typename ScalarT, int ColumnSize, int RowSize>
	inline matrix_<ScalarT, ColumnSize, RowSize> operator * (float s, matrix_<ScalarT, ColumnSize, RowSize> const& lhs){
		return lhs * s;
	}

	template <typename ScalarT, int ColumnSize, int RowSize>
	inline matrix_<ScalarT, ColumnSize, RowSize> operator / (matrix_<ScalarT, ColumnSize, RowSize> const& lhs, float s){
		matrix_<ScalarT, ColumnSize, RowSize> ret;
		for(int i = 0; i < RowSize; ++i)
		{
			for(int j = 0; j < ColumnSize; ++j)
			{
				ret.data_[i][j] = lhs.data_[i][j] / s;
			}
		}
		return ret;
	}

	template <typename ScalarT, int ColumnSize, int RowSize>
	inline matrix_<ScalarT, ColumnSize, RowSize> operator / (float s, matrix_<ScalarT, ColumnSize, RowSize> const& lhs){
		matrix_<ScalarT, ColumnSize, RowSize> ret;
		for(int i = 0; i < RowSize; ++i)
		{
			for(int j = 0; j < ColumnSize; ++j)
			{
				ret.data_[i][j] = s / lhs.data_[i][j];
			}
		}
		return ret;
	}
}

#endif