#include <sasl/syntax_tree/expression.h>
#include <sasl/syntax_tree/visitor.h>

using std::shared_ptr;

namespace sasl::syntax_tree {

operator_traits::operator_traits() {
  prefix_ops =
      decltype(prefix_ops){operators::positive,  operators::negative,    operators::bit_not,
                           operators::logic_not, operators::prefix_incr, operators::prefix_decr};

  postfix_ops = decltype(postfix_ops){operators::postfix_incr, operators::postfix_decr};

  binary_ops = decltype(binary_ops){operators::add,
                                    operators::sub,
                                    operators::mul,
                                    operators::div,
                                    operators::mod,
                                    operators::assign,
                                    operators::add_assign,
                                    operators::sub_assign,
                                    operators::mul_assign,
                                    operators::div_assign,
                                    operators::mod_assign,
                                    operators::bit_and,
                                    operators::bit_or,
                                    operators::bit_xor,
                                    operators::bit_and_assign,
                                    operators::bit_or_assign,
                                    operators::bit_xor_assign,
                                    operators::logic_and,
                                    operators::logic_or,
                                    operators::equal,
                                    operators::not_equal,
                                    operators::greater,
                                    operators::greater_equal,
                                    operators::less,
                                    operators::less_equal,
                                    operators::left_shift,
                                    operators::right_shift,
                                    operators::lshift_assign,
                                    operators::rshift_assign};
}

bool operator_traits::is_prefix(operators op) { return include(prefix_ops, op); }

bool operator_traits::is_binary(operators op) { return include(binary_ops, op); }

bool operator_traits::is_postfix(operators op) { return include(postfix_ops, op); }

bool operator_traits::is_unary(operators op) { return is_prefix(op) || is_postfix(op); }

bool operator_traits::include(const std::vector<operators> &c, operators op) {
  return std::find(c.begin(), c.end(), op) != c.end();
}

} // namespace sasl::syntax_tree
