#pragma once

#include <sasl/codegen/forward.h>
#include <vector>

namespace llvm {
class Function;
class Module;
class LLVMContext;
class FunctionType;
} // namespace llvm

namespace sasl::codegen {

class llvm_intrin_cache {
public:
  llvm_intrin_cache();
  llvm::Function *get(char const *, llvm::Module *);
  llvm::Function *get(int, llvm::Module *);
  llvm::Function *get(int, llvm::Module *, llvm::FunctionType *);

private:
  std::vector<llvm::Function *> intrin_fns;
};

} // namespace sasl::codegen