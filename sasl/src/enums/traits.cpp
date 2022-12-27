#include <sasl/enums/builtin_types.h>
#include <sasl/enums/operators.h>
#include <sasl/enums/traits.h>

#include <eflib/math/math.h>
#include <eflib/utility/enum.h>

#include <mutex>

namespace sasl::enums::traits {

std::mutex mtx_btlist_init;
std::vector<builtin_types> btc_list;

const std::vector<builtin_types> &list_of_builtin_types() {
  std::scoped_lock<std::mutex> locker(mtx_btlist_init);
  if (btc_list.empty()) {
    // add scalars.
    btc_list =
        decltype(btc_list){builtin_types::_sint8,  builtin_types::_sint16, builtin_types::_sint32,
                           builtin_types::_sint64, builtin_types::_uint8,  builtin_types::_uint16,
                           builtin_types::_uint32, builtin_types::_uint64, builtin_types::_boolean,
                           builtin_types::_float,  builtin_types::_double};

    // add vectors & matrixs
    size_t scalar_count = btc_list.size();
    for (size_t i_scalar = 0; i_scalar < scalar_count; ++i_scalar) {
      for (int i = 1; i <= 4; ++i) {
        for (int j = 1; j <= 4; ++j) {
          btc_list.push_back(matrix_of(btc_list[i_scalar], i, j));
        }
        btc_list.push_back(vector_of(btc_list[i_scalar], i));
      }
    }

    // add other types.
    btc_list.push_back(builtin_types::none);
    btc_list.push_back(builtin_types::_void);
    btc_list.push_back(builtin_types::_sampler);
  }
  return btc_list;
}

bool is_arithmetic(const operators &op) {
  return op == operators::add || op == operators::sub || op == operators::mul ||
         op == operators::div || op == operators::mod;
}

bool is_relationship(const operators &op) {
  return op == operators::greater || op == operators::greater_equal || op == operators::equal ||
         op == operators::less || op == operators::less_equal || op == operators::not_equal;
}

bool is_bit(const operators &op) {
  return op == operators::bit_and || op == operators::bit_or || op == operators::bit_xor;
}

bool is_shift(const operators &op) {
  return op == operators::left_shift || op == operators::right_shift;
}

bool is_bool_arith(const operators &op) {
  return op == operators::logic_and || op == operators::logic_or;
}

bool is_prefix(const operators &op) {
  return op == operators::prefix_decr || op == operators::prefix_incr;
}

bool is_postfix(const operators &op) {
  return op == operators::postfix_decr || op == operators::postfix_incr;
}

bool is_unary_arith(const operators &op) {
  return op == operators::positive || op == operators::negative;
}

bool is_arith_assign(const operators &op) {
  return op == operators::add_assign || op == operators::sub_assign ||
         op == operators::mul_assign || op == operators::div_assign || op == operators::mod_assign;
}

bool is_bit_assign(const operators &op) {
  return op == operators::bit_and_assign || op == operators::bit_or_assign ||
         op == operators::bit_xor_assign;
}

bool is_shift_assign(const operators &op) {
  return op == operators::lshift_assign || op == operators::rshift_assign;
}

bool is_assign(const operators &op) { return op == operators::assign; }

bool is_general_assign(operators const &op) {
  return is_assign(op) || is_arith_assign(op) || is_bit_assign(op) || is_shift_assign(op);
}

std::mutex mtx_oplist_init;
std::vector<operators> op_list;

const std::vector<operators> &list_of_operators() {
  std::scoped_lock<std::mutex> locker(mtx_oplist_init);
  if (op_list.empty()) {
    op_list = decltype(op_list){operators::add,
                                operators::add_assign,
                                operators::assign,
                                operators::bit_and,
                                operators::bit_and_assign,
                                operators::bit_not,
                                operators::bit_or,
                                operators::bit_or_assign,
                                operators::bit_xor,
                                operators::bit_xor_assign,
                                operators::div,
                                operators::div_assign,
                                operators::equal,
                                operators::greater,
                                operators::greater_equal,
                                operators::left_shift,
                                operators::less,
                                operators::less_equal,
                                operators::logic_and,
                                operators::logic_or,
                                operators::logic_not,
                                operators::lshift_assign,
                                operators::mod,
                                operators::mod_assign,
                                operators::mul,
                                operators::mul_assign,
                                operators::negative,
                                operators::none,
                                operators::not_equal,
                                operators::positive,
                                operators::postfix_decr,
                                operators::postfix_incr,
                                operators::prefix_decr,
                                operators::prefix_incr,
                                operators::right_shift,
                                operators::rshift_assign,
                                operators::sub,
                                operators::sub_assign};
  }
  return op_list;
}

} // namespace sasl::enums::traits
