#pragma once

#include <sasl/enums/jump_mode.h>
#include <sasl/syntax_tree/node.h>
#include <sasl/syntax_tree/syntax_tree_fwd.h>

#include <memory>
#include <variant>
#include <vector>

namespace sasl::syntax_tree {

using sasl::common::token;

struct compound_statement;
struct declaration;
struct expression;
struct identifier;
struct label;

struct statement: public node_impl<statement, node, node_ids::statement> {};

struct labeled_statement: public node_impl<labeled_statement, statement, node_ids::labeled_statement> {
  std::shared_ptr<struct label> pop_label();
  void push_label(std::shared_ptr<label> lbl);

  std::shared_ptr<statement> stmt;
  std::vector<std::shared_ptr<struct label>> labels;
};

struct declaration_statement: public node_impl<declaration_statement, statement, node_ids::declaration_statement> {
  std::vector<std::shared_ptr<declaration>> decls;
};

struct if_statement: public node_impl<if_statement, statement, node_ids::if_statement> {
  std::shared_ptr<expression> cond;
  std::shared_ptr<statement> yes_stmt, no_stmt;
};

struct while_statement: public node_impl<while_statement, statement, node_ids::while_statement> {
  std::shared_ptr<expression> cond;
  std::shared_ptr<statement> body;
};

struct dowhile_statement: public node_impl<dowhile_statement, statement, node_ids::dowhile_statement> {
  std::shared_ptr<statement> body;
  std::shared_ptr<expression> cond;
};

struct for_statement: public node_impl<for_statement, statement, node_ids::for_statement> {
  std::shared_ptr<statement> init;
  std::shared_ptr<expression> cond;
  std::shared_ptr<expression> iter;
  std::shared_ptr<compound_statement> body;
};

struct label: public node_impl<label, node, node_ids::label> {};

struct case_label: public node_impl<case_label, label, node_ids::case_label> {
  // if expr is null pointer, it means default.
  std::shared_ptr<expression> expr;
};

struct ident_label: public node_impl<ident_label, label, node_ids::ident_label> {
  token label_tok;
};

struct switch_statement: public node_impl<switch_statement, statement, node_ids::switch_statement> {
  std::shared_ptr<expression> cond;
  std::shared_ptr<compound_statement> stmts;
};

struct compound_statement: public node_impl<compound_statement, statement, node_ids::compound_statement> {
  std::vector<std::shared_ptr<statement>> stmts;
};

struct expression_statement: public node_impl<expression_statement, statement, node_ids::expression_statement> {
  std::shared_ptr<expression> expr;
};

struct jump_statement: public node_impl<jump_statement, statement, node_ids::jump_statement> {
  jump_mode code;
  std::shared_ptr<expression> jump_expr; // for return only
};

} // namespace sasl::syntax_tree
