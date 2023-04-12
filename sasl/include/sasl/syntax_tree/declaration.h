#pragma once

#include <sasl/enums/builtin_types.h>
#include <sasl/enums/node_ids.h>
#include <sasl/enums/type_qualifiers.h>

#include <sasl/common/token.h>
#include <sasl/syntax_tree/node.h>
#include <sasl/syntax_tree/syntax_tree_fwd.h>

#include <memory>
#include <vector>

namespace sasl::syntax_tree {

using token = sasl::common::token;
class syntax_tree_visitor;

EFLIB_DECLARE_STRUCT_SHARED_PTR(tynode);
EFLIB_DECLARE_STRUCT_SHARED_PTR(compound_statement);
EFLIB_DECLARE_STRUCT_SHARED_PTR(statement);
EFLIB_DECLARE_STRUCT_SHARED_PTR(expression);
EFLIB_DECLARE_STRUCT_SHARED_PTR(function_type);

struct initializer : public node_impl<initializer, node, node_ids::initializer> {};

struct expression_initializer
  : public node_impl<expression_initializer, initializer, node_ids::expression_initializer> {
  std::shared_ptr<expression> init_expr;
};

struct member_initializer
  : public node_impl<member_initializer, initializer, node_ids::member_initializer> {
  std::vector<std::shared_ptr<initializer>> sub_inits;
};

EFLIB_DECLARE_STRUCT_SHARED_PTR(declaration);
struct declaration : public node_impl<declaration, node, node_ids::declaration> {};

struct declarator : public node_impl<declarator, node, node_ids::declarator> {
  token name;
  std::shared_ptr<initializer> init;
  token semantic;
  token semantic_index;
};

struct variable_declaration
  : public node_impl<variable_declaration, declaration, node_ids::variable_declaration> {
  std::shared_ptr<tynode> type_info;
  std::vector<std::shared_ptr<declarator>> declarators;
};

struct type_definition
  : public node_impl<type_definition, declaration, node_ids::typedef_definition> {
  std::shared_ptr<tynode> type_info;
  token name;
};

struct tynode : public node_impl<tynode, declaration, node_ids::tynode> {
  builtin_types tycode;
  type_qualifiers qual;

  bool is_builtin() const;
  bool is_struct() const;
  bool is_array() const;
  bool is_function() const;
  bool is_alias() const;

  bool is_uniform() const;
};

struct alias_type : public node_impl<alias_type, tynode, node_ids::alias_type> {
  token alias;
};

struct builtin_type : public node_impl<builtin_type, tynode, node_ids::builtin_type> {
public:
  bool is_builtin() const { return true; }
};

EFLIB_DECLARE_STRUCT_SHARED_PTR(array_type);
struct array_type : public node_impl<array_type, tynode, node_ids::array_type> {
  std::vector<std::shared_ptr<expression>> array_lens;
  std::shared_ptr<tynode> elem_type;
};

struct struct_type : public node_impl<struct_type, tynode, node_ids::struct_type> {
  token name;
  bool has_body;
  std::vector<std::shared_ptr<declaration>> decls;
};

struct parameter_full : public node_impl<parameter_full, declaration, node_ids::parameter_full> {
  std::shared_ptr<tynode> param_type;
  token name;
  std::shared_ptr<initializer> init;
  token semantic;
  token semantic_index;
};

struct function_full_def
  : public node_impl<function_full_def, tynode, node_ids::function_full_def> {
  token name;
  std::shared_ptr<tynode> retval_type;
  std::vector<std::shared_ptr<parameter_full>> params;
  token semantic;
  token semantic_index;
  std::shared_ptr<compound_statement> body;

  bool declaration_only();
};

struct parameter : public node_impl<parameter, declaration, node_ids::parameter> {
  token name;
  std::shared_ptr<initializer> init;
  token semantic;
  token semantic_index;
};

struct function_def : public node_impl<function_def, declaration, node_ids::function_def> {
  std::shared_ptr<function_type> type;

  token name;
  std::vector<std::shared_ptr<parameter>> params;
  token semantic;
  token semantic_index;

  std::shared_ptr<compound_statement> body;

  bool declaration_only();
};

struct function_type : public node_impl<function_type, tynode, node_ids::function_type> {
  std::vector<tynode_ptr> param_types;
  tynode_ptr result_type;
};

struct null_declaration
  : public node_impl<null_declaration, declaration, node_ids::null_declaration> {};
}  // namespace sasl::syntax_tree
