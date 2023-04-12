#pragma once

#include <eflib/platform/config.h>

#include <eflib/platform/stdint.h>

#include <type_traits>

namespace eflib {
template <typename ScalarT, int ColumnSize, int RowSize, int ColumnStride = 4>
struct matrix_;

template <typename Type>
struct is_matrix_type : std::false_type {};

template <typename ScalarT, int ColumnSize, int RowSize, int ColumnStride>
struct is_matrix_type<matrix_<ScalarT, ColumnSize, RowSize, ColumnStride>> : std::true_type {};

template <typename T>
inline constexpr bool is_matrix_v = is_matrix_type<T>::value;

template <typename ScalarT, int ColumnSize, int RowSize, int ColumnStride = 4>
struct matrix_data;

template <typename ScalarT, int Size>
struct vector_;

template <typename ScalarT, int ColumnSize, int RowSize>
struct det_ {};

template <typename ScalarT>
struct det_<ScalarT, 4, 4> {
  ScalarT det() const {
    auto const* derived_this = static_cast<matrix_<ScalarT, 4, 4> const*>(this);

    ScalarT _3142_3241(derived_this->data_[2][0] * derived_this->data_[3][1] -
                       derived_this->data_[2][1] * derived_this->data_[3][0]);
    ScalarT _3143_3341(derived_this->data_[2][0] * derived_this->data_[3][2] -
                       derived_this->data_[2][2] * derived_this->data_[3][0]);
    ScalarT _3144_3441(derived_this->data_[2][0] * derived_this->data_[3][3] -
                       derived_this->data_[2][3] * derived_this->data_[3][0]);
    ScalarT _3243_3342(derived_this->data_[2][1] * derived_this->data_[3][2] -
                       derived_this->data_[2][2] * derived_this->data_[3][1]);
    ScalarT _3244_3442(derived_this->data_[2][1] * derived_this->data_[3][3] -
                       derived_this->data_[2][3] * derived_this->data_[3][1]);
    ScalarT _3344_3443(derived_this->data_[2][2] * derived_this->data_[3][3] -
                       derived_this->data_[2][3] * derived_this->data_[3][2]);

    return derived_this->data_[0][0] *
        (derived_this->data_[1][1] * _3344_3443 - derived_this->data_[1][2] * _3244_3442 +
         derived_this->data_[1][3] * _3243_3342) -
        derived_this->data_[0][1] *
        (derived_this->data_[1][0] * _3344_3443 - derived_this->data_[1][2] * _3144_3441 +
         derived_this->data_[1][3] * _3143_3341) +
        derived_this->data_[0][2] *
        (derived_this->data_[1][0] * _3244_3442 - derived_this->data_[1][1] * _3144_3441 +
         derived_this->data_[1][3] * _3142_3241) -
        derived_this->data_[0][3] *
        (derived_this->data_[1][0] * _3243_3342 - derived_this->data_[1][1] * _3143_3341 +
         derived_this->data_[1][2] * _3142_3241);
  }
};

template <typename ScalarT, int ColumnSize, int RowSize, int ColumnStride>
struct matrix_data {
  static_assert(ColumnStride >= ColumnSize);

  using scalar_type = ScalarT;
  static constexpr auto column_size = ColumnSize;
  static constexpr auto column_stride = ColumnStride;
  static constexpr auto row_size = RowSize;

  using iterator = ScalarT*;
  using const_iterator = ScalarT const*;

  ScalarT data_[RowSize][ColumnStride];

  template <typename IndexT>
  vector_<ScalarT, RowSize> get_column(IndexT i) const {
    vector_<ScalarT, RowSize> ret;
    for (int i_row = 0; i_row < RowSize; ++i_row) {
      ret[i_row] = data_[i_row][i];
    }
    return ret;
  }

  template <typename IndexT>
  void set_column(IndexT i, vector_<ScalarT, RowSize> const& v) {
    for (int i_row = 0; i_row < RowSize; ++i_row) {
      data_[i_row][i] = v[i_row];
    }
  }

  template <typename IndexT>
  vector_<ScalarT, ColumnSize> const& get_row(IndexT i) const {
    return *reinterpret_cast<vector_<ScalarT, ColumnSize> const*>(&(data_[i][0]));
  }

  template <typename IndexT>
  void set_row(IndexT i, vector_<ScalarT, ColumnSize> const& v) {
    for (int i_col = 0; i_col < ColumnSize; ++i_col) {
      data_[i][i_col] = v[i_col];
    }
  }

  iterator begin() { return &data_[0][0]; }
  const_iterator begin() const { return &data_[0][0]; }

  iterator end() { return begin() + RowSize * ColumnSize; }
  const_iterator end() const { return begin() + RowSize * ColumnSize; }
};

template <typename ScalarT, int ColumnSize, int RowSize, int ColumnStride, bool IsSquare>
struct matrix_operators : public det_<ScalarT, ColumnSize, RowSize> {
  using matrix_type = matrix_<ScalarT, ColumnSize, RowSize, ColumnStride>;

  static matrix_type zero() {
    matrix_type ret;
    for (size_t i = 0; i < ColumnStride * RowSize; ++i) {
      (&ret.data_[0][0])[i] = ScalarT(0);
    }
    return ret;
  }
};

template <typename ScalarT, int ColumnSize, int RowSize, int ColumnStride>
struct matrix_operators<ScalarT, ColumnSize, RowSize, ColumnStride, true>
  : public matrix_operators<ScalarT, ColumnSize, RowSize, ColumnStride, false> {
  using this_type = matrix_operators<ScalarT, ColumnSize, RowSize, ColumnStride, true>;
  using matrix_type = matrix_<ScalarT, ColumnSize, RowSize, ColumnStride>;
  static matrix_type diag(std::initializer_list<ScalarT> values) {
    return diag(values.begin(), values.end());
  }

  template <typename ScalarIt>
  static matrix_type diag(ScalarIt begin, ScalarIt end) {
    matrix_type ret = this_type::zero();
    for (size_t i = 0; i < ColumnSize && begin != end; ++i, ++begin) {
      ret.data_[i][i] = *begin;
    }
    return ret;
  }

  static matrix_type identity() {
    constexpr auto s = static_cast<ScalarT>(1);
    return diag({s, s, s, s});
  }
};

template <typename ScalarT, int ColumnSize, int RowSize, int ColumnStride>
struct matrix_
  : matrix_operators<ScalarT, ColumnSize, RowSize, ColumnStride, ColumnSize == RowSize>
  , matrix_data<ScalarT, ColumnSize, RowSize, ColumnStride> {
  using this_type = matrix_<ScalarT, ColumnSize, RowSize, ColumnStride>;
  template <int ColumnStride2>
  using assignable_type = matrix_<ScalarT, ColumnSize, RowSize, ColumnStride2>;

  matrix_() = default;

  template <int ColumnStride2>
  explicit matrix_(assignable_type<ColumnStride2> const& v) {
    for (size_t i = 0; i < RowSize; ++i) {
      for (size_t j = 0; j < ColumnSize; ++j) {
        this->data_[i][j] = v.data_[i][j];
      }
    }
  }
};

template <typename ScalarT>
struct matrix_<ScalarT, 4, 4, 4>
  : matrix_operators<ScalarT, 4, 4, 4, true>
  , matrix_data<ScalarT, 4, 4, 4> {
  matrix_() = default;

  matrix_(matrix_ const& v) = default;
  matrix_& operator=(matrix_ const& v) = default;

  matrix_(ScalarT _11,
          ScalarT _12,
          ScalarT _13,
          ScalarT _14,
          ScalarT _21,
          ScalarT _22,
          ScalarT _23,
          ScalarT _24,
          ScalarT _31,
          ScalarT _32,
          ScalarT _33,
          ScalarT _34,
          ScalarT _41,
          ScalarT _42,
          ScalarT _43,
          ScalarT _44) {
    this->data_[0][0] = _11;
    this->data_[0][1] = _12;
    this->data_[0][2] = _13;
    this->data_[0][3] = _14;

    this->data_[1][0] = _21;
    this->data_[1][1] = _22;
    this->data_[1][2] = _23;
    this->data_[1][3] = _24;

    this->data_[2][0] = _31;
    this->data_[2][1] = _32;
    this->data_[2][2] = _33;
    this->data_[2][3] = _34;

    this->data_[3][0] = _41;
    this->data_[3][1] = _42;
    this->data_[3][2] = _43;
    this->data_[3][3] = _44;
  }
};

// Element-wise operators
template <typename MatrixType, typename = std::enable_if<is_matrix_v<MatrixType>>>
inline MatrixType operator+(MatrixType const& lhs, MatrixType const& rhs) {
  MatrixType ret;
  for (int i = 0; i < MatrixType::row_size; ++i) {
    for (int j = 0; j < MatrixType::column_size; ++j) {
      ret.data_[i][j] = lhs.data_[i][j] + rhs.data_[i][j];
    }
  }
  return ret;
}

template <typename MatrixType, typename = std::enable_if<is_matrix_v<MatrixType>>>
inline MatrixType operator-(MatrixType const& lhs, MatrixType const& rhs) {
  MatrixType ret;
  for (int i = 0; i < MatrixType::row_size; ++i) {
    for (int j = 0; j < MatrixType::column_size; ++j) {
      ret.data_[i][j] = lhs.data_[i][j] - rhs.data_[i][j];
    }
  }
  return ret;
}

template <typename MatrixType, typename = std::enable_if<is_matrix_v<MatrixType>>>
inline MatrixType operator*(MatrixType const& lhs, MatrixType const& rhs) {
  MatrixType ret;
  for (int i = 0; i < MatrixType::row_size; ++i) {
    for (int j = 0; j < MatrixType::column_size; ++j) {
      ret.data_[i][j] = lhs.data_[i][j] * rhs.data_[i][j];
    }
  }
  return ret;
}

template <typename MatrixType, typename = std::enable_if<is_matrix_v<MatrixType>>>
inline MatrixType operator/(MatrixType const& lhs, MatrixType const& rhs) {
  MatrixType ret;
  for (int i = 0; i < MatrixType::row_size; ++i) {
    for (int j = 0; j < MatrixType::column_size; ++j) {
      ret.data_[i][j] = lhs.data_[i][j] / rhs.data_[i][j];
    }
  }
  return ret;
}

// Matrix op Scalar
template <typename MatrixType, typename = std::enable_if<is_matrix_v<MatrixType>>>
inline MatrixType operator+(MatrixType const& lhs, typename MatrixType::scalar_type s) {
  MatrixType ret;
  for (int i = 0; i < MatrixType::row_size; ++i) {
    for (int j = 0; j < MatrixType::column_size; ++j) {
      ret.data_[i][j] = lhs.data_[i][j] + s;
    }
  }
  return ret;
}

template <typename MatrixType, typename = std::enable_if<is_matrix_v<MatrixType>>>
inline MatrixType operator-(MatrixType const& lhs, typename MatrixType::scalar_type s) {
  MatrixType ret;
  for (int i = 0; i < MatrixType::row_size; ++i) {
    for (int j = 0; j < MatrixType::column_size; ++j) {
      ret.data_[i][j] = lhs.data_[i][j] - s;
    }
  }
  return ret;
}

template <typename MatrixType, typename = std::enable_if<is_matrix_v<MatrixType>>>
inline MatrixType operator*(MatrixType const& lhs, typename MatrixType::scalar_type s) {
  MatrixType ret;
  for (int i = 0; i < MatrixType::row_size; ++i) {
    for (int j = 0; j < MatrixType::column_size; ++j) {
      ret.data_[i][j] = lhs.data_[i][j] * s;
    }
  }
  return ret;
}

template <typename MatrixType, typename = std::enable_if<is_matrix_v<MatrixType>>>
inline MatrixType operator/(MatrixType const& lhs, typename MatrixType::scalar_type s) {
  MatrixType ret;
  for (int i = 0; i < MatrixType::row_size; ++i) {
    for (int j = 0; j < MatrixType::column_size; ++j) {
      ret.data_[i][j] = lhs.data_[i][j] / s;
    }
  }
  return ret;
}

// Scalar op Matrix
template <typename MatrixType, typename = std::enable_if<is_matrix_v<MatrixType>>>
inline MatrixType operator+(typename MatrixType::scalar_type s, MatrixType const& lhs) {
  return lhs + s;
}

template <typename MatrixType, typename = std::enable_if<is_matrix_v<MatrixType>>>
inline MatrixType operator*(typename MatrixType::scalar_type s, MatrixType const& lhs) {
  return lhs * s;
}

template <typename MatrixType, typename = std::enable_if<is_matrix_v<MatrixType>>>
inline MatrixType operator-(typename MatrixType::scalar_type s, MatrixType const& lhs) {
  MatrixType ret;
  for (int i = 0; i < MatrixType::row_size; ++i) {
    for (int j = 0; j < MatrixType::column_size; ++j) {
      ret.data_[i][j] = s - lhs.data_[i][j];
    }
  }
  return ret;
}

template <typename MatrixType, typename = std::enable_if<is_matrix_v<MatrixType>>>
inline MatrixType operator/(typename MatrixType::scalar_type s, MatrixType const& lhs) {
  MatrixType ret;
  for (int i = 0; i < MatrixType::row_size; ++i) {
    for (int j = 0; j < MatrixType::column_size; ++j) {
      ret.data_[i][j] = s / lhs.data_[i][j];
    }
  }
  return ret;
}

template <typename MatrixType, typename = std::enable_if<is_matrix_v<MatrixType>>>
inline MatrixType operator-(MatrixType const& lhs, float s) {
  MatrixType ret;
  for (int i = 0; i < MatrixType::row_size; ++i) {
    for (int j = 0; j < MatrixType::column_size; ++j) {
      ret.data_[i][j] = lhs.data_[i][j] - s;
    }
  }
  return ret;
}
}  // namespace eflib
