#include <sasl/drivers/drivers_api.h>

#include <sasl/drivers/compiler_lib.h>

#include <sasl/shims/ia_shim.h>
#include <sasl/shims/interp_shim.h>

#include <eflib/concurrency/atomic.h>

#include <llvm/Support/ManagedStatic.h>
#include <llvm/Support/TargetSelect.h>


class llvm_initializer {
public:
  llvm_initializer() {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    // llvm::InitializeNativeTargetAsmParser();
  }

  ~llvm_initializer() { llvm::llvm_shutdown(); }

  static llvm_initializer &initialize() {
    static llvm_initializer obj;
    return obj;
  }
};

void sasl_create_compiler(sasl::drivers::compiler_ptr &out) {
  llvm_initializer::initialize();
  out = sasl::drivers::create_compiler();
}

void sasl_create_ia_shim(sasl::shims::ia_shim_ptr &out) {
  llvm_initializer::initialize();
  out = sasl::shims::ia_shim::create();
}

void sasl_create_interp_shim(sasl::shims::interp_shim_ptr &out) {
  llvm_initializer::initialize();
  out = sasl::shims::interp_shim::create();
}