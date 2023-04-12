#pragma once

#include <sasl/enums/builtin_types.h>
#include <sasl/enums/literal_classifications.h>
#include <sasl/enums/node_ids.h>
#include <sasl/enums/operators.h>
#include <sasl/enums/type_qualifiers.h>

#include <sasl/common/token.h>

#include <eflib/utility/composition.h>

#include <optional>

namespace sasl::syntax_tree_next {

using token = sasl::common::token;
using namespace eflib::composition;

struct node_base {
  token token_beg, token_end;
};

struct indexed_semantic {
  token semantic;
  token index;
};

// ... forward declarations ...
struct expression;
struct statement;
struct qualified_type;
struct declaration;

// ... expressions ...

struct constant_expression {
  node_base base;
  token value_tok;
  literal_classifications ctype;
};

struct variable_expression {
  node_base base;
  token var_name;
};

struct unary_expression {
  node_base base;
  indirect_<expression> primary_expr;
  token op_token;
  operators op;
};

struct binary_expression {
  node_base base;
  indirect_<expression> left_oprand;
  indirect_<expression> right_operand;
  token op_token;
  operators op;
};

struct cast_expression {
  node_base base;
  indirect_<qualified_type> dest_type;
  indirect_<expression> expr;
};

struct expression_list {
  node_base base;
  std::vector<expression> exprs;
};

struct cond_expression {
  node_base base;
  indirect_<expression> cond;
  indirect_<expression> pos;
  indirect_<expression> neg;
};

struct subscript_expression {
  node_base base;
  indirect_<expression> expr;
  indirect_<expression> sub;
};

struct call_expression {
  node_base base;
  indirect_<expression> expr;
  std::vector<expression> args;
};

struct member_expression {
  node_base base;
  indirect_<expression> expr;
  token member;
};

struct expression
  : std::variant<constant_expression,
                 variable_expression,
                 unary_expression,
                 binary_expression,
                 cast_expression,
                 expression_list,
                 cond_expression,
                 subscript_expression,
                 call_expression,
                 member_expression> {};

// ... statements ...

struct empty_statement {
  node_base base;
};

struct declaration_statement {
  node_base base;
  indirect_<declaration> decl;
};

struct expression_statement {
  node_base base;
  expression expr;
};

struct continue_statement {
  node_base base;
};

struct break_statement {
  node_base base;
};

struct return_statement {
  node_base base;
  std::optional<expression> expr;
};

struct if_statement {
  node_base base;
  expression cond;
  indirect_<statement> pos_stmt, neg_stmt;
};

struct while_statement {
  node_base base;
  expression cond;
  indirect_<statement> body;
};

struct dowhile_statement {
  node_base base;
  indirect_<statement> body;
  expression cond;
};

struct for_statement {
  node_base base;
  declaration_statement init;
  expression cond, iter;
  indirect_<statement> body;
};

struct switch_statement {
  node_base base;
  expression expr;
  indirect_<statement> body;
};

struct compound_statement {
  node_base base;
  std::vector<statement> stmts;
};

struct statement
  : std::variant<empty_statement,
                 declaration_statement,
                 expression_statement,
                 continue_statement,
                 break_statement,
                 return_statement,
                 if_statement,
                 while_statement,
                 dowhile_statement,
                 for_statement,
                 switch_statement,
                 compound_statement> {};

// ... declarations ...

struct initializer;

struct expression_initializer {
  node_base base;
  expression init_expr;
};

struct aggregate_initializer {
  node_base base;
  std::vector<initializer> sub_inits;
};

struct initializer : std::variant<expression_initializer, aggregate_initializer> {};

struct declarator {
  node_base base;
  token name;
  initializer init;
  std::optional<indexed_semantic> semantic;
};

struct array_type {
  node_base base;
  std::vector<expression> dims;
  indirect_<qualified_type> element_type;
};

struct alias_type {
  node_base base;
  indirect_<qualified_type> type;
  token alias;
};

struct builtin_type {
  node_base base;
  builtin_types code;
};

struct parameter {
  node_base base;
  indirect_<qualified_type> type;
  token name;
  std::optional<initializer> init;
  std::optional<indexed_semantic> semantic;
};

struct function_signature {
  node_base base;
  token name;
  std::vector<parameter> params;

  struct result_t {
    indirect_<qualified_type> type;
    token semantic;
    token semantic_index;
  } result;
};

struct struct_definition {
  node_base base;
  token name;
  std::vector<declaration> decls;
};

struct unqualified_type
  : std::variant<builtin_type, array_type, alias_type, function_signature, struct_definition> {};

struct qualified_type {
  node_base base;
  type_qualifiers quals;
  unqualified_type prim;
};

struct variable_declaration {
  node_base base;
  qualified_type type;
  std::vector<declarator> declarators;
};

struct function_definition {
  node_base base;
  function_signature signature;
  compound_statement body;
};

struct elaborated_type {
  node_base base;
  token name;
};

struct declaration
  : std::variant<variable_declaration,
                 function_definition,
                 function_signature,
                 struct_definition,
                 elaborated_type> {};

struct program {
  std::vector<declaration> decls;
};

}  // namespace sasl::syntax_tree_next