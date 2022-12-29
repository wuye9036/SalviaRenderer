#pragma once

#include <sasl/codegen/forward.h>

#include <sasl/codegen/cg_sisd.h>
#include <sasl/semantic/reflector.h>
#include <sasl/syntax_tree/visitor.h>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include <any>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace sasl {
namespace semantic {
class caster_t;
class module_semantic;
} // namespace semantic
namespace syntax_tree {
struct expression;
struct tynode;
struct node;
} // namespace syntax_tree
} // namespace sasl

namespace llvm {
class Constant;
}

enum class builtin_types : uint32_t;

namespace sasl::codegen {

class module_vmcode_impl;
class module_vmcode;

class cg_general : public cg_sisd {
public:
  typedef cg_sisd parent_class;

  cg_general();

  using cg_sisd::visit;

  SASL_VISIT_DCL(expression_list);

  SASL_VISIT_BASE_DCL(identifier);

  // declaration & type specifier
  SASL_VISIT_DCL(initializer);

  SASL_VISIT_DCL(member_initializer);
  SASL_VISIT_DCL(type_definition);
  SASL_VISIT_DCL(tynode);
  SASL_VISIT_DCL(alias_type);

  // statement
protected:
  SASL_SPECIFIC_VISIT_DCL(before_decls_visit, program) override;
  SASL_SPECIFIC_VISIT_DCL(bin_logic, binary_expression) override;

private:
  module_vmcode_impl *mod_ptr();
};

} // namespace sasl::codegen