#pragma once

#include <sasl/enums/builtin_types.h>
#include <sasl/enums/operators.h>

#include <eflib/platform/typedefs.h>
#include <eflib/utility/enum.h>

#include <salvia/shader/constants.h>
#include <salvia/shader/reflection.h>

#include <cassert>
#include <string>
#include <type_traits>
#include <vector>

namespace sasl::enums {
using namespace eflib::enum_operators;

constexpr bool is_none(builtin_types btc) noexcept { return btc == builtin_types::none; }

constexpr bool is_void(builtin_types btc) noexcept { return btc == builtin_types::_void; }

constexpr bool is_sampler(builtin_types btc) noexcept { return btc == builtin_types::_sampler; }

constexpr bool is_integer(builtin_types btc) noexcept {
  return (btc & builtin_types::_generic_type_mask) == builtin_types::_integer;
}

constexpr bool is_real(builtin_types btc) noexcept {
  return (btc & builtin_types::_generic_type_mask) == builtin_types::_real;
}

constexpr bool is_signed(builtin_types btc) noexcept {
  return (btc & builtin_types::_sign_mask) == builtin_types::_signed;
}

constexpr bool is_unsigned(builtin_types btc) noexcept {
  return (btc & builtin_types::_sign_mask) == builtin_types::_unsigned;
}

constexpr bool is_scalar(builtin_types btc) noexcept {
  bool scalar = ((btc & builtin_types::_dimension_mask) == builtin_types::_scalar);
  return scalar && !is_void(btc) && !is_none(btc) && !is_sampler(btc);
}

constexpr bool is_numeric(builtin_types btc) noexcept { return is_integer(btc) || is_real(btc); }

constexpr bool is_numeric_scalar(builtin_types btc) noexcept {
  return is_scalar(btc) && is_numeric(btc);
}

constexpr bool is_vector(builtin_types btc) noexcept {
  return (btc & builtin_types::_dimension_mask) == builtin_types::_vector;
}

constexpr bool is_matrix(builtin_types btc) noexcept {
  return (btc & builtin_types::_dimension_mask) == builtin_types::_matrix;
}

constexpr bool is_storagable(builtin_types btc) noexcept {
  return is_scalar(btc) || is_vector(btc) || is_matrix(btc);
}

constexpr builtin_types scalar_of(builtin_types btc) noexcept {
  return (btc & builtin_types::_scalar_type_mask);
}

constexpr builtin_types vector_of(builtin_types btc, size_t len) noexcept {
  if (!is_scalar(btc)) {
    return builtin_types::none;
  }
  builtin_types ret = (btc | builtin_types::_vector) |
                      static_cast<builtin_types>(len << builtin_types::_dim0_field_shift);
  return ret;
}

constexpr builtin_types matrix_of(builtin_types btc, size_t vec_size, size_t vec_cnt) noexcept {
  if (!is_scalar(btc)) {
    return builtin_types::none;
  }
  builtin_types ret = (btc | builtin_types::_matrix) |
                      static_cast<builtin_types>(vec_size << builtin_types::_dim0_field_shift) |
                      static_cast<builtin_types>(vec_cnt << builtin_types::_dim1_field_shift);

  return ret;
}

constexpr size_t vector_size(builtin_types btc) noexcept {
  if (is_sampler(btc) || is_scalar(btc)) {
    return 1;
  }
  return static_cast<size_t>(btc & builtin_types::_dim0_mask) >> builtin_types::_dim0_field_shift;
}

constexpr size_t vector_count(builtin_types btc) noexcept {
  if (is_sampler(btc) || is_scalar(btc) || is_vector(btc)) {
    return 1;
  }
  return static_cast<size_t>(btc & builtin_types::_dim1_mask) >> builtin_types::_dim1_field_shift;
}

constexpr builtin_types row_vector_of(builtin_types btc) noexcept {
  if (!is_matrix(btc)) {
    return builtin_types::none;
  }

  return vector_of(scalar_of(btc), vector_size(btc));
}

constexpr size_t reg_storage_size(builtin_types btc) noexcept {
  if (is_none(btc) || is_void(btc)) {
    return 0;
  }

  builtin_types s_btc = scalar_of(btc);

  size_t comp_size = 0;

  switch (s_btc) {
  case builtin_types::_sint32:
  case builtin_types::_uint32:
  case builtin_types::_float:
  case builtin_types::_boolean:
    comp_size = 4;
    break;
  case builtin_types::_sampler:
    comp_size = sizeof(void *);
    break;
  default:
    break;
  }

  if (is_matrix(btc)) {
    return vector_count(btc) * comp_size * 4;
  } else if (is_vector(btc)) {
    return vector_size(btc) * comp_size;
  }
  return comp_size;
}

constexpr size_t storage_size(builtin_types btc) noexcept {
  if (is_none(btc) || is_void(btc)) {
    return 0;
  }
  if (is_sampler(btc)) {
    return sizeof(void *);
  }
  size_t component_count = vector_size(btc) * vector_count(btc);
  size_t component_size = 0;
  builtin_types s_btc = scalar_of(btc);
  if (s_btc == builtin_types::_sint8 || s_btc == builtin_types::_uint8 ||
      s_btc == builtin_types::_boolean) {
    component_size = 1;
  } else if (s_btc == builtin_types::_sint16 || s_btc == builtin_types::_uint16) {
    component_size = 2;
  } else if (s_btc == builtin_types::_sint32 || s_btc == builtin_types::_uint32 ||
             s_btc == builtin_types::_float || s_btc == builtin_types::_boolean) {
    component_size = 4;
  } else if (s_btc == builtin_types::_sint64 || s_btc == builtin_types::_uint64 ||
             s_btc == builtin_types::_double) {
    component_size = 8;
  }

  return component_size * component_count;
}

const std::vector<builtin_types> &list_of_builtin_types();

//////////////////////////////////////////////////////////////////////////
// operators
// types
bool is_arithmetic(const operators &);
bool is_relationship(const operators &);
bool is_bit(const operators &);
bool is_shift(const operators &);
bool is_bool_arith(const operators &);

// operand count
bool is_prefix(const operators &);
bool is_postfix(const operators &);
bool is_unary_arith(const operators &);

// is assign?
bool is_arith_assign(const operators &);
bool is_bit_assign(const operators &);
bool is_shift_assign(const operators &);
bool is_assign(const operators &);
bool is_general_assign(const operators &);

const std::vector<operators> &list_of_operators();

constexpr bool is_standard(builtin_types btc) noexcept {
  if (btc == builtin_types::_sint32 || btc == builtin_types::_uint32 ||
      btc == builtin_types::_sint64 || btc == builtin_types::_uint64 ||
      btc == builtin_types::_float || btc == builtin_types::_double) {
    return true;
  }

  if (is_vector(btc) || is_matrix(btc)) {
    return is_standard(scalar_of(btc));
  }

  return false;
}

constexpr builtin_types replace_scalar(builtin_types btc, builtin_types scalar_btc) noexcept {
  if (!is_scalar(scalar_btc)) {
    return scalar_btc;
  }

  if (is_vector(btc)) {
    return vector_of(scalar_btc, vector_size(btc));
  } else if (is_matrix(btc)) {
    return matrix_of(scalar_btc, vector_size(btc), vector_count(btc));
  } else {
    return scalar_btc;
  }
}

constexpr builtin_types to_builtin_types(salvia::shader::language_value_types v) noexcept {
  return static_cast<builtin_types>(v);
}

constexpr salvia::shader::language_value_types to_lvt(builtin_types v) noexcept {
  return static_cast<salvia::shader::language_value_types>(v);
}

constexpr bool operator==(builtin_types lhs, salvia::shader::language_value_types rhs) noexcept {
  return to_lvt(lhs) == rhs;
}
constexpr bool operator==(salvia::shader::language_value_types lhs, builtin_types rhs) noexcept {
  return rhs == lhs;
}
constexpr bool operator!=(salvia::shader::language_value_types lhs, builtin_types rhs) noexcept {
  return !(rhs == lhs);
}
constexpr bool operator!=(builtin_types lhs, salvia::shader::language_value_types rhs) noexcept {
  return !(lhs == rhs);
}

} // namespace sasl::enums