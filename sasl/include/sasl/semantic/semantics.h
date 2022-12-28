#ifndef SASL_SEMANTIC_SEMANTICS_H
#define SASL_SEMANTIC_SEMANTICS_H

#include <sasl/semantic/semantic_forward.h>

#include <eflib/platform/typedefs.h>
#include <eflib/utility/shared_declaration.h>
#include <salvia/shader/constants.h>
#include <salvia/shader/reflection.h>
#include <sasl/enums/builtin_types.h>
#include <sasl/enums/literal_classifications.h>
#include <sasl/semantic/elem_indexes.h>

#include <memory>
#include <string>
#include <vector>

namespace salviar {
class semantic_value;
}

namespace sasl {
namespace syntax_tree {
struct tynode;
struct labeled_statement;
EFLIB_DECLARE_STRUCT_SHARED_PTR(node);
EFLIB_DECLARE_STRUCT_SHARED_PTR(program);
} // namespace syntax_tree
namespace common {
EFLIB_DECLARE_CLASS_SHARED_PTR(diag_chat);
}
} // namespace sasl

namespace sasl::semantic {

class node_semantic;
class pety_t;
EFLIB_DECLARE_CLASS_SHARED_PTR(symbol);
EFLIB_DECLARE_CLASS_SHARED_PTR(module_semantic);

class module_semantic {
public:
  static module_semantic_ptr create();

  virtual ~module_semantic() {}

  virtual salvia::shader::languages get_language() const = 0;
  virtual void set_language(salvia::shader::languages lang) = 0;

  virtual symbol *root_symbol() const = 0;

  virtual sasl::syntax_tree::program_ptr get_program() const = 0;
  virtual void set_program(sasl::syntax_tree::program_ptr const &) = 0;

  virtual pety_t *pety() const = 0;
  virtual sasl::common::diag_chat_ptr diags() const = 0;

  virtual std::vector<symbol *> const &global_vars() const = 0;
  virtual std::vector<symbol *> &global_vars() = 0;

  virtual bool is_modified(symbol *) const = 0;
  virtual void modify(symbol *) = 0;

  virtual std::vector<symbol *> const &functions() const = 0;
  virtual std::vector<symbol *> &functions() = 0;

  virtual std::vector<symbol *> const &intrinsics() const = 0;
  virtual std::vector<symbol *> &intrinsics() = 0;

  template <typename T> node_semantic *get_semantic(std::shared_ptr<T> const &v) {
    return get_semantic(v.get());
  }

  template <typename T> node_semantic *get_or_create_semantic(std::shared_ptr<T> const &v) {
    return get_or_create_semantic(v.get());
  }

  template <typename T> node_semantic *create_semantic(std::shared_ptr<T> const &v) {
    return create_semantic(v.get());
  }

  virtual node_semantic *get_semantic(sasl::syntax_tree::node const *) const = 0;
  virtual node_semantic *create_semantic(sasl::syntax_tree::node const *) = 0;
  virtual node_semantic *get_or_create_semantic(sasl::syntax_tree::node const *) = 0;

  virtual void hold_generated_node(sasl::syntax_tree::node_ptr const &) = 0;
  virtual symbol *get_symbol(sasl::syntax_tree::node *) const = 0;
  virtual symbol *alloc_symbol() = 0; ///< Only called by symbol.
  virtual void link_symbol(sasl::syntax_tree::node *, symbol *) = 0;
};

namespace lvalue_or_rvalue {
enum id { unknown, lvalue = 1 << 0, rvalue = 1 << 1, lrvalue = lvalue | rvalue };
}

class node_semantic {
public:
  typedef std::vector<std::weak_ptr<sasl::syntax_tree::labeled_statement>> labeled_statement_array;

  ~node_semantic();

  node_semantic(node_semantic const &) = delete;
  node_semantic &operator=(node_semantic const &);

  // Read functions
public:
  // General
  [[nodiscard]] module_semantic *owner() const { return owner_; }
  [[nodiscard]] sasl::syntax_tree::node *associated_node() const { return assoc_node_; }
  [[nodiscard]] symbol *associated_symbol() const { return assoc_symbol_; }

  // Type
  [[nodiscard]] int tid() const { return tid_; }
  [[nodiscard]] builtin_types value_builtin_type() const;
  [[nodiscard]] sasl::syntax_tree::tynode *ty_proto() const;

  // Constant
  [[nodiscard]] int64_t const_signed() const { return signed_constant_; }
  [[nodiscard]] uint64_t const_unsigned() const { return unsigned_constant_; }
  [[nodiscard]] double const_double() const { return double_constant_; }
  [[nodiscard]] std::string const_string() const;

  // Expression and variable
  [[nodiscard]] salvia::shader::semantic_value *semantic_value() const { return semantic_value_; }
  [[nodiscard]] salvia::shader::semantic_value const &semantic_value_ref() const;
  [[nodiscard]] salvia::shader::reg_name user_defined_reg() const { return user_defined_reg_; }
  [[nodiscard]] int member_index() const { return member_index_; }
  [[nodiscard]] elem_indexes swizzle() const { return swizzle_code_; }
  [[nodiscard]] bool is_reference() const { return is_reference_; }
  [[nodiscard]] bool is_function_pointer() const { return is_function_pointer_; }
  [[nodiscard]] lvalue_or_rvalue::id lr_value() const { return lrv_; }
  [[nodiscard]] bool is_modified() const { return modified_; }
  [[nodiscard]] size_t member_offset() const { return member_offset_; }

  // Function and intrinsic
  [[nodiscard]] std::string const &function_name() const;
  [[nodiscard]] symbol *overloaded_function() const { return overloaded_function_; }
  [[nodiscard]] bool is_intrinsic() const { return is_intrinsic_; }
  [[nodiscard]] bool is_external() const { return is_external_; }
  [[nodiscard]] bool msc_compatible() const { return msc_compatible_; }
  [[nodiscard]] bool is_invoked() const { return is_invoked_; }
  [[nodiscard]] bool partial_execution() const { return partial_execution_; }
  [[nodiscard]] bool is_constructor() const { return is_constructor_; }
  [[nodiscard]] sasl::syntax_tree::node *referenced_declarator() const { return referenced_declarator_; }
  // Statement
  [[nodiscard]] labeled_statement_array const &labeled_statements() const;
  [[nodiscard]] bool has_loop() const { return has_loop_; }

  // Write functions
public:
  // General
  void owner(module_semantic *v) { owner_ = v; }

  void associated_node(sasl::syntax_tree::node *v) { assoc_node_ = v; }
  void associated_symbol(symbol *v) { assoc_symbol_ = v; }

  // Type
  void tid(int v);
  void internal_tid(int v, sasl::syntax_tree::tynode *proto); /// Only used by pety.
  void ty_proto(sasl::syntax_tree::tynode *ty, symbol *scope);

  // Constant
  void const_value(std::string const &lit, literal_classifications lit_class);
  void const_value(int64_t v) { signed_constant_ = v; }
  void const_value(uint64_t v) { unsigned_constant_ = v; }
  void const_value(std::string const &v);
  void const_value(double v) { double_constant_ = v; }

  // Expression and variable
  void semantic_value(salvia::shader::semantic_value const &v);
  void lr_value(lvalue_or_rvalue::id lrv) { lrv_ = lrv; }
  void member_index(int v) { member_index_ = v; }
  void swizzle(elem_indexes const &v) { swizzle_code_ = v; }
  void is_reference(bool v) { is_reference_ = v; }
  void is_function_pointer(bool v) { is_function_pointer_ = v; }
  void modify_value() { modified_ = true; }
  void referenced_declarator(sasl::syntax_tree::node *v) { referenced_declarator_ = v; }
  void member_offset(size_t offset) { member_offset_ = offset; }

  // Function and intrinsic
  void function_name(std::string const &v);
  void overloaded_function(symbol *v) { overloaded_function_ = v; }
  void is_intrinsic(bool v) { is_intrinsic_ = v; }
  void msc_compatible(bool v) { msc_compatible_ = v; }
  void is_invoked(bool v) { is_invoked_ = v; }
  void partial_execution(bool v) { partial_execution_ = v; }
  void is_constructor(bool v) { is_constructor_ = v; }

  // Statement
  labeled_statement_array &labeled_statements();
  void has_loop(bool v) { has_loop_ = v; }

private:
  sasl::syntax_tree::node *assoc_node_{};
  module_semantic *owner_{};
  symbol *assoc_symbol_{};

  // Type
  sasl::syntax_tree::tynode *proto_type_{};
  int tid_{};

  // Constant
  int64_t signed_constant_{};
  uint64_t unsigned_constant_{};
  std::string *string_constant_{};
  double double_constant_{};

  // Expression and variable
  sasl::syntax_tree::node *referenced_declarator_{};
  salvia::shader::semantic_value *semantic_value_{};
  salvia::shader::reg_name user_defined_reg_;
  int member_index_{};
  elem_indexes swizzle_code_;
  bool is_reference_{};
  bool is_function_pointer_{};
  bool modified_{};
  lvalue_or_rvalue::id lrv_;
  size_t member_offset_{};

  // Function and intrinsic
  std::string *function_name_{};
  symbol *overloaded_function_{};
  bool is_intrinsic_{};
  bool is_invoked_{};
  bool msc_compatible_{};
  bool is_external_{};
  bool partial_execution_{};
  bool is_constructor_{};

  // Statement
  labeled_statement_array *labeled_statements_{};
  bool has_loop_{};
};

} // namespace sasl::semantic

#endif