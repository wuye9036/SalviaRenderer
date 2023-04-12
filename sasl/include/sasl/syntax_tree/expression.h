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
  bool include(const std::vector<operators>&, operators);
  operator_traits();
  typedef ::std::vector<operators> oplist_t;
  oplist_t prefix_ops, postfix_ops, binary_ops;
};

using sasl::common::token;
struct tynode;
class syntax_tree_visitor;

struct expression : public node_impl<expression, node, node_ids::expression> {};

struct constant_expression
  : public node_impl<constant_expression, expression, node_ids::constant_expression> {
  token value_tok;
  literal_classifications ctype;
};

struct variable_expression
  : public node_impl<variable_expression, expression, node_ids::variable_expression> {
  token var_name;
};

struct unary_expression
  : public node_impl<unary_expression, expression, node_ids::unary_expression> {
  std::shared_ptr<expression> expr;
  token op_token;
  operators op;
};

struct cast_expression : public node_impl<cast_expression, expression, node_ids::cast_expression> {
  std::shared_ptr<tynode> casted_type;
  std::shared_ptr<expression> expr;
};

struct binary_expression
  : public node_impl<binary_expression, expression, node_ids::binary_expression> {
  operators op;
  token op_token;
  std::shared_ptr<expression> left_expr;
  std::shared_ptr<expression> right_expr;
};

struct expression_list : public node_impl<expression_list, expression, node_ids::expression_list> {
  std::vector<std::shared_ptr<expression>> exprs;
};

struct cond_expression : public node_impl<cond_expression, expression, node_ids::cond_expression> {
  std::shared_ptr<expression> cond_expr;
  std::shared_ptr<expression> yes_expr;
  std::shared_ptr<expression> no_expr;
};

struct index_expression
  : public node_impl<index_expression, expression, node_ids::index_expression> {
  std::shared_ptr<expression> expr;
  std::shared_ptr<expression> index_expr;
};

struct call_expression : public node_impl<call_expression, expression, node_ids::call_expression> {
  std::shared_ptr<expression> expr;
  std::vector<std::shared_ptr<expression>> args;
};

struct member_expression
  : public node_impl<member_expression, expression, node_ids::member_expression> {
  std::shared_ptr<expression> expr;
  token member;
};

}  // namespace sasl::syntax_tree
