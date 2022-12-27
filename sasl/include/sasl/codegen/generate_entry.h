#pragma once

#include <sasl/codegen/forward.h>
#include <vector>
namespace llvm {
class Type;
}

namespace sasl::semantic {
class reflection_impl;
} // namespace sasl::semantic

namespace sasl::codegen {

class cg_service;

std::vector<llvm::Type *> generate_vs_entry_param_type(sasl::semantic::reflection_impl const *abii,
                                                       cg_service *cg);
std::vector<llvm::Type *> generate_ps_entry_param_type(sasl::semantic::reflection_impl const *abii,
                                                       cg_service *cg);
} // namespace sasl::codegen