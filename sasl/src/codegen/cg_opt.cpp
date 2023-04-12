#include <llvm/Analysis/Passes.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/raw_os_ostream.h>

#include <sasl/codegen/cg_api.h>

#include <memory>
#include <vector>

using llvm::Function;
using llvm::Module;
using llvm::raw_os_ostream;

using std::shared_ptr;

using std::ostream;
using std::vector;

namespace sasl::codegen {

#if TODO
void optimize(shared_ptr<module_vmcode> code, vector<optimization_options> opt_options) {
  Module* mod = code->get_vm_module();

  FunctionPassManager fpm(mod);

  for (optimization_options opt_option : opt_options) {
    switch (opt_option) {
    case opt_verify:
      for (Function& f : mod->getFunctionList()) {
        if (!f.empty()) {
          verifyFunction(f, PrintMessageAction);
        }
      }
      break;
    case opt_preset_std_for_function:
      // createStandardFunctionPasses( &fpm, 1 );
      break;
    }
  }

  fpm.doInitialization();

  for (Function& f : mod->getFunctionList()) {
    if (!f.empty()) {
      fpm.run(f);
    }
  }
}
#endif

}  // namespace sasl::codegen
