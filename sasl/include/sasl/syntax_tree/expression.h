#pragma once

#include <sasl/enums/literal_classifications.h>
#include <sasl/enums/operators.h>
#include <sasl/syntax_tree/node.h>
#include <sasl/syntax_tree/syntax_tree_fwd.h>

#include <memory>
#include <string>
#include <vector>

namespace sasl::syntax_tree {

class operator_traits {
public:
  bool is_prefix(operators op);
  bool is_binary(operators op);
  bool is_postfix(operators op);
  bool is_unary(operators op);

private:
  bool include(const std::vector<operators> &, operators);
  operator_traits();
  typedef ::std::vector<operators> oplist_t;
  oplist_t prefix_ops, postfix_ops, binary_ops;
};

using sasl::common::token;
struct tynode;
class syntax_tree_visitor;

struct expression : public node {
};

struct constant_expression : public expression {
  token value_tok;
  literal_classifications ctype;
};

struct variable_expression : public expression {
  token var_name;
};

struct unary_expression : public expression {
  std::shared_ptr<expression> expr;
  token op_token;
  operators op;
};

struct cast_expression : public expression {
  std::shared_ptr<tynode> casted_type;
  std::shared_ptr<expression> expr;
};

struct binary_expression : public expression {
  operators op;
  token op_token;
  std::shared_ptr<expression> left_expr;
  std::shared_ptr<expression> right_expr;
};

struct expression_list : public expression {
  std::vector<std::shared_ptr<expression>> exprs;
};

struct cond_expression : public expression {
  std::shared_ptr<expression> cond_expr;
  std::shared_ptr<expression> yes_expr;
  std::shared_ptr<expression> no_expr;
};

struct index_expression : public expression {
  std::shared_ptr<expression> expr;
  std::shared_ptr<expression> index_expr;
};

struct call_expression : public expression {
  std::shared_ptr<expression> expr;
  std::vector<std::shared_ptr<expression>> args;
};

struct member_expression : public expression{
  std::shared_ptr<expression> expr;
  token member;
};

} // namespace sasl::syntax_tree
