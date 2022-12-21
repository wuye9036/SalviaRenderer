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

struct statement : public node {};

struct labeled_statement : public statement {
  std::shared_ptr<struct label> pop_label();
  void push_label(std::shared_ptr<label> lbl);

  std::shared_ptr<statement> stmt;
  std::vector<std::shared_ptr<struct label>> labels;
};

struct declaration_statement : public statement {
  std::vector<std::shared_ptr<declaration>> decls;
};

struct if_statement : public statement {
  std::shared_ptr<expression> cond;
  std::shared_ptr<statement> yes_stmt, no_stmt;
};

struct while_statement : public statement {
  std::shared_ptr<expression> cond;
  std::shared_ptr<statement> body;
};

struct dowhile_statement : public statement {
  std::shared_ptr<statement> body;
  std::shared_ptr<expression> cond;
};

struct for_statement : public statement {
  std::shared_ptr<statement> init;
  std::shared_ptr<expression> cond;
  std::shared_ptr<expression> iter;
  std::shared_ptr<compound_statement> body;
};

struct label : public node {};

struct case_label : public label {
  // if expr is null pointer, it means default.
  std::shared_ptr<expression> expr;
};

struct ident_label : public label {
  token label_tok;
};

struct switch_statement : public statement {
  std::shared_ptr<expression> cond;
  std::shared_ptr<compound_statement> stmts;
};

struct compound_statement : public statement {
  std::vector<std::shared_ptr<statement>> stmts;
};

struct expression_statement : public statement {
  std::shared_ptr<expression> expr;
};

struct jump_statement : public statement {
  jump_mode code;
  std::shared_ptr<expression> jump_expr; // for return only
};

} // namespace sasl::syntax_tree
