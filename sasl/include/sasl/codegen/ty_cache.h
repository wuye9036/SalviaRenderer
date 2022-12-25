#pragma once

#include <sasl/codegen/forward.h>

#include <sasl/codegen/cgs.h>
#include <sasl/enums/builtin_types.h>

#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Type.h>

#include <string>
#include <vector>

namespace sasl::codegen {

using namespace llvm;
using std::string;

void initialize_cache(LLVMContext &ctxt);
Type *get_llvm_type(LLVMContext &ctxt, builtin_types bt, abis abi);

} // namespace sasl::codegen