#pragma once

#include <memory>
#include <sasl/codegen/cg_sisd.h>
#include <sasl/semantic/reflection_impl.h>

namespace sasl {
namespace semantic {
class caster_t;
class module_semantic;
}  // namespace semantic
namespace syntax_tree {
struct expression;
struct tynode;
struct node;
}  // namespace syntax_tree
}  // namespace sasl

namespace llvm {
class PointerType;
class StructType;
class DataLayout;
}  // namespace llvm

namespace sasl::codegen {

class cg_vs : public cg_sisd {
public:
  typedef cg_sisd parent_class;

  cg_vs();
  ~cg_vs();

  using cg_sisd::visit;

  // expressions
  SASL_VISIT_DCL(member_expression);
  SASL_VISIT_DCL(cast_expression);
  SASL_VISIT_DCL(expression_list);
  SASL_VISIT_DCL(cond_expression);

  SASL_VISIT_DCL(variable_expression);
  SASL_VISIT_BASE_DCL(identifier);

  // declaration & type specifier
  SASL_VISIT_DCL(initializer);
  SASL_VISIT_DCL(member_initializer);
  SASL_VISIT_DCL(declaration);
  SASL_VISIT_DCL(type_definition);
  SASL_VISIT_DCL(tynode);
  SASL_VISIT_DCL(alias_type);

private:
  SASL_SPECIFIC_VISIT_DCL(before_decls_visit, program) override;

  // Binary logical operators.
  SASL_SPECIFIC_VISIT_DCL(bin_logic, binary_expression) override;

  // Overrides them for generating entry function if need.
  SASL_SPECIFIC_VISIT_DCL(create_fnsig, function_def) override;
  SASL_SPECIFIC_VISIT_DCL(create_fnargs, function_def) override;
  SASL_SPECIFIC_VISIT_DCL(create_virtual_args, function_def);

  SASL_SPECIFIC_VISIT_DCL(visit_return, jump_statement) override;

  bool is_entry(llvm::Function*) const;

  module_vmcode_impl* mod_ptr();

  multi_value layout_to_value(salvia::shader::sv_layout* si, bool copy_from_input);

  // If ctxt is nullptr, the generated value and type will be cached.
  // Return true if context is fetched from cache.
  bool layout_to_node_context(node_context* ctxt,
                              salvia::shader::sv_layout* si,
                              bool store_to_existed_value,
                              bool copy_from_input);

  void copy_to_result(std::shared_ptr<sasl::syntax_tree::expression> const&);
  void copy_to_agg_result(node_context* data);

  llvm::Function* entry_fn;
  sasl::semantic::symbol* entry_sym;

  multi_value param_values[salvia::shader::sv_usage_count];

  typedef std::unordered_map<salvia::shader::semantic_value, node_context*> input_copies_dict;
  input_copies_dict input_copies_;
};

}  // namespace sasl::codegen