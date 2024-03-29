#include <sasl/codegen/cg_intrins.h>

#include <llvm/ADT/StringRef.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/Module.h>

#include <vector>

using llvm::Function;
using llvm::LLVMContext;
using llvm::Module;
using llvm::StringRef;
using std::vector;

namespace Intrinsic = llvm::Intrinsic;

namespace sasl::codegen {

llvm::Intrinsic::ID get_intrinsic_id(char const* Name) {
  unsigned Len = static_cast<unsigned>(strlen(Name));

  if (Len < 5 || Name[4] != '.' || Name[0] != 'l' || Name[1] != 'l' || Name[2] != 'v' ||
      Name[3] != 'm')
    return llvm::Intrinsic::ID(0);  // All intrinsics start with 'llvm.'

  return llvm::Intrinsic::ID(0);
}

llvm_intrin_cache::llvm_intrin_cache() : intrin_fns(Intrinsic::num_intrinsics) {
}

Function* llvm_intrin_cache::get(char const* name, Module* mod) {
  return get(get_intrinsic_id(name), mod);
}

Function* llvm_intrin_cache::get(int id, Module* mod) {
  llvm::Intrinsic::ID IID = llvm::Intrinsic::ID(id);
  assert(!Intrinsic::isOverloaded(IID));

  if (intrin_fns[IID] == nullptr) {
    intrin_fns[IID] = ((IID == 0) ? nullptr : Intrinsic::getDeclaration(mod, IID));
  }
  return intrin_fns[IID];
}

Function* llvm_intrin_cache::get(int id, Module* mod, llvm::FunctionType* fnty) {
  llvm::Intrinsic::ID IID = llvm::Intrinsic::ID(id);
  vector<llvm::Type*> par_types;
  for (unsigned i = 0; i < fnty->getNumParams(); ++i) {
    par_types.push_back(fnty->getParamType(i));
  }
  return llvm::cast<Function>(Intrinsic::getDeclaration(mod, IID, par_types));
}

}  // namespace sasl::codegen
