#pragma once

#include <sasl/codegen/cg_impl.h>

#include <salvia/shader/reflection.h>
#include <sasl/codegen/cgs_simd.h>
#include <sasl/enums/builtin_types.h>

namespace sasl {
namespace semantic {
class module_semantic;
class reflection_impl;
} // namespace semantic
} // namespace sasl

namespace llvm {
class StructType;
class Type;
} // namespace llvm

namespace sasl::codegen {

// Code generation for SIMD( Single Instruction Multiple Data )
class cg_simd : public cg_impl {

public:
  typedef cg_impl parent_class;

  cg_simd();
  ~cg_simd();

  using cg_impl::visit;

  // expression
  SASL_VISIT_DCL(unary_expression);
  SASL_VISIT_DCL(cast_expression);
  SASL_VISIT_DCL(expression_list);
  SASL_VISIT_DCL(cond_expression);
  SASL_VISIT_DCL(index_expression);
  SASL_VISIT_DCL(member_expression);
  SASL_VISIT_DCL(variable_expression);

  // declaration & type specifier
  SASL_VISIT_DCL(initializer);
  SASL_VISIT_DCL(member_initializer);
  SASL_VISIT_DCL(declaration);
  SASL_VISIT_DCL(type_definition);
  SASL_VISIT_DCL(tynode);
  SASL_VISIT_DCL(array_type);
  SASL_VISIT_DCL(alias_type);

  // statement
  SASL_VISIT_DCL(statement);
  SASL_VISIT_DCL(if_statement);
  SASL_VISIT_DCL(while_statement);
  SASL_VISIT_DCL(dowhile_statement);
  SASL_VISIT_DCL(for_statement);
  SASL_VISIT_DCL(case_label);
  SASL_VISIT_DCL(ident_label);
  SASL_VISIT_DCL(switch_statement);
  SASL_VISIT_DCL(compound_statement);
  SASL_VISIT_DCL(labeled_statement);

protected:
  cgs_simd *service() const override;
  abis local_abi(bool is_c_compatible) const override;

  multi_value layout_to_value(salvia::shader::sv_layout *svl);

  SASL_SPECIFIC_VISIT_DCL(before_decls_visit, program) override;

  SASL_SPECIFIC_VISIT_DCL(create_fnsig, function_def) override;
  SASL_SPECIFIC_VISIT_DCL(create_fnargs, function_def) override;
  SASL_SPECIFIC_VISIT_DCL(create_virtual_args, function_def);

  SASL_SPECIFIC_VISIT_DCL(visit_return, jump_statement) override;
  SASL_SPECIFIC_VISIT_DCL(visit_continue, jump_statement) override;
  SASL_SPECIFIC_VISIT_DCL(visit_break, jump_statement) override;

  SASL_SPECIFIC_VISIT_DCL(bin_logic, binary_expression) override;

  llvm::Function *entry_fn;
  multi_value entry_values[salvia::shader::sv_usage_count];
};

} // namespace sasl::codegen