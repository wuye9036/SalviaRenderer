#pragma once

#include <sasl/enums/builtin_types.h>
#include <sasl/enums/node_ids.h>
#include <sasl/enums/type_qualifiers.h>

#include <sasl/common/token.h>
#include <sasl/syntax_tree/node.h>
#include <sasl/syntax_tree/syntax_tree_fwd.h>

#include <eflib/utility/enable_if.h>

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

struct initializer : public node {};

struct expression_initializer : public initializer {
  std::shared_ptr<expression> init_expr;
};

struct member_initializer : public initializer {
  std::vector<std::shared_ptr<initializer>> sub_inits;
};

EFLIB_DECLARE_STRUCT_SHARED_PTR(declaration);
struct declaration : public node {};

struct declarator : public node {
  token name;
  std::shared_ptr<initializer> init;
  token semantic;
  token semantic_index;
};

struct variable_declaration : public declaration {
  std::shared_ptr<tynode> type_info;
  std::vector<std::shared_ptr<declarator>> declarators;
};

struct type_definition : public declaration {
  std::shared_ptr<tynode> type_info;
  token name;
};

struct tynode : public declaration {
  builtin_types tycode;
  type_qualifiers qual;

  bool is_builtin() const;
  bool is_struct() const;
  bool is_array() const;
  bool is_function() const;
  bool is_alias() const;

  bool is_uniform() const;
};

struct alias_type : public tynode {
  token alias;
};

struct builtin_type : public tynode {
public:
  bool is_builtin() const { return true; }
};

EFLIB_DECLARE_STRUCT_SHARED_PTR(array_type);
struct array_type : public tynode {
  std::vector<std::shared_ptr<expression>> array_lens;
  std::shared_ptr<tynode> elem_type;
};

struct struct_type : public tynode {
  token  name;
  bool has_body;
  std::vector<std::shared_ptr<declaration>> decls;
};

struct parameter_full : public declaration {
  std::shared_ptr<tynode> param_type;
  token name;
  std::shared_ptr<initializer> init;
  token semantic;
  token semantic_index;
};

struct function_full_def : public tynode {
  token  name;
  std::shared_ptr<tynode> retval_type;
  std::vector<std::shared_ptr<parameter_full>> params;
  token semantic;
  token semantic_index;
  std::shared_ptr<compound_statement> body;

  bool declaration_only();
};

struct parameter : public declaration {
  token name;
  std::shared_ptr<initializer> init;
  token semantic;
  token semantic_index;
};

struct function_def : public declaration {
  std::shared_ptr<function_type> type;

  token name;
  std::vector<std::shared_ptr<parameter>> params;
  token semantic;
  token semantic_index;

  std::shared_ptr<compound_statement> body;

  bool declaration_only();
};

struct function_type : public tynode {
  std::vector<tynode_ptr> param_types;
  tynode_ptr result_type;
};

struct null_declaration : public declaration {
};
} // namespace sasl::syntax_tree
