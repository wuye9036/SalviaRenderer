#pragma once

#include <sasl/codegen/cg_api.h>
#include <sasl/codegen/forward.h>
#include <sasl/semantic/reflection_impl.h>

namespace llvm {
class LLVMContext;
class Module;
class ExecutionEngine;
class Function;
class Type;

class IRBuilderDefaultInserter;
template <typename T, typename Inserter> class IRBuilder;
class ConstantFolder;
using DefaultIRBuilder = IRBuilder<ConstantFolder, IRBuilderDefaultInserter>;
} // namespace llvm

namespace sasl {
namespace semantic {
EFLIB_DECLARE_CLASS_SHARED_PTR(module_semantic);
}
} // namespace sasl

#include <boost/shared_ptr.hpp>
#include <string>

namespace sasl::codegen {

EFLIB_DECLARE_CLASS_SHARED_PTR(module_context);

// module_vmcode_impl contains all LLVM related objects which are used by JIT.
class module_vmcode_impl : public module_vmcode {
public:
  module_vmcode_impl(std::string_view module_name);

  virtual sasl::semantic::module_semantic *get_semantic() const;
  virtual void set_semantic(sasl::semantic::module_semantic_ptr const &);
  virtual module_context *get_context() const;
  virtual void set_context(module_context_ptr const &);

  virtual void *get_function(std::string_view);
  virtual void inject_function(void *, std::string_view);

  virtual llvm::Module *get_vm_module() const;
  virtual llvm::LLVMContext &get_vm_context();
  virtual llvm::DefaultIRBuilder *builder() const;

  virtual void dump_ir() const;
  virtual void dump_ir(std::ostream &ostr) const;

  ~module_vmcode_impl();

protected:
  std::unique_ptr<llvm::LLVMContext> vm_ctx_;
  std::unique_ptr<llvm::ExecutionEngine> vm_engine_;
  std::unique_ptr<llvm::DefaultIRBuilder> ir_builder_;
  llvm::Module *vm_module_;

  sasl::semantic::module_semantic_ptr sem_;
  module_context_ptr ctxt_;
  std::string error_;
  bool finalized_;
};

} // namespace sasl::codegen
