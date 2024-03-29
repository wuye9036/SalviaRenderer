#pragma once

#include <sasl/enums/builtin_types.h>
#include <sasl/enums/operators.h>

#include <eflib/platform/stdint.h>
#include <eflib/utility/enum.h>

#include <salvia/shader/constants.h>
#include <salvia/shader/reflection.h>

#include <cassert>
#include <string>
#include <type_traits>
#include <vector>

namespace sasl::enums {
using namespace eflib::enum_operators;

constexpr bool is_none(builtin_types btc) noexcept {
  return btc == builtin_types::none;
}

constexpr bool is_void(builtin_types btc) noexcept {
  return btc == builtin_types::_void;
}

constexpr bool is_sampler(builtin_types btc) noexcept {
  return btc == builtin_types::_sampler;
}

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

constexpr bool is_numeric(builtin_types btc) noexcept {
  return is_integer(btc) || is_real(btc);
}

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
  case builtin_types::_boolean: comp_size = 4; break;
  case builtin_types::_sampler: comp_size = sizeof(void*); break;
  default: break;
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
    return sizeof(void*);
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

const std::vector<builtin_types>& list_of_builtin_types();

//////////////////////////////////////////////////////////////////////////
// operators
// types
constexpr bool is_arithmetic(const operators& op) {
  return op == operators::add || op == operators::sub || op == operators::mul ||
      op == operators::div || op == operators::mod;
}

constexpr bool is_relationship(const operators& op) {
  return op == operators::greater || op == operators::greater_equal || op == operators::equal ||
      op == operators::less || op == operators::less_equal || op == operators::not_equal;
}

constexpr bool is_bit(const operators& op) {
  return op == operators::bit_and || op == operators::bit_or || op == operators::bit_xor;
}

constexpr bool is_shift(const operators& op) {
  return op == operators::left_shift || op == operators::right_shift;
}

constexpr bool is_bool_arith(const operators& op) {
  return op == operators::logic_and || op == operators::logic_or;
}

constexpr bool is_prefix(const operators& op) {
  return op == operators::prefix_decr || op == operators::prefix_incr;
}

constexpr bool is_postfix(const operators& op) {
  return op == operators::postfix_decr || op == operators::postfix_incr;
}

constexpr bool is_unary_arith(const operators& op) {
  return op == operators::positive || op == operators::negative;
}

constexpr bool is_arith_assign(const operators& op) {
  return op == operators::add_assign || op == operators::sub_assign ||
      op == operators::mul_assign || op == operators::div_assign || op == operators::mod_assign;
}

constexpr bool is_bit_assign(const operators& op) {
  return op == operators::bit_and_assign || op == operators::bit_or_assign ||
      op == operators::bit_xor_assign;
}

constexpr bool is_shift_assign(const operators& op) {
  return op == operators::lshift_assign || op == operators::rshift_assign;
}

constexpr bool is_assign(const operators& op) {
  return op == operators::assign;
}

constexpr bool is_general_assign(operators const& op) {
  return is_assign(op) || is_arith_assign(op) || is_bit_assign(op) || is_shift_assign(op);
}

template <typename T>
inline constexpr T all_operators() {
  return T{operators::add,           operators::sub,
           operators::mul,           operators::div,
           operators::mod,           operators::greater,
           operators::greater_equal, operators::equal,
           operators::less,          operators::less_equal,
           operators::not_equal,     operators::bit_and,
           operators::bit_or,        operators::bit_xor,
           operators::left_shift,    operators::right_shift,
           operators::logic_and,     operators::logic_or,
           operators::prefix_decr,   operators::prefix_incr,
           operators::postfix_decr,  operators::postfix_incr,
           operators::positive,      operators::negative,
           operators::add_assign,    operators::sub_assign,
           operators::mul_assign,    operators::div_assign,
           operators::mod_assign,    operators::bit_and_assign,
           operators::bit_or_assign, operators::bit_xor_assign,
           operators::lshift_assign, operators::rshift_assign,
           operators::assign};
}

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

}  // namespace sasl::enums