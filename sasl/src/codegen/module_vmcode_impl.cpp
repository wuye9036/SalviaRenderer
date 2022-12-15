#include <sasl/codegen/module_vmcode_impl.h>

#include <sasl/semantic/reflector.h>

#include <eflib/platform/cpuinfo.h>

#include <eflib/platform/disable_warnings.h>
#include <eflib/platform/enable_warnings.h>
#include <llvm/ADT/Triple.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/DynamicLibrary.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_os_ostream.h>

#include <string>
#include <string_view>
#include <vector>

using sasl::semantic::module_semantic;
using std::shared_ptr;
using std::string;
using std::string_view;
using std::vector;

struct llvm_options {
  llvm_options() {
    // Add Options
    char const *options[] = {"" /*, "-force-align-stack"*/};
    llvm::cl::ParseCommandLineOptions(sizeof(options) / sizeof(char *), options);
  }

  static llvm_options &initialize() {
    static llvm_options opt;
    return opt;
  }
};

namespace sasl::codegen {

module_vmcode_impl::module_vmcode_impl(string_view name) {
  vm_ctx_ = std::make_unique<llvm::LLVMContext>();
  ir_builder_ = std::make_unique<llvm::DefaultIRBuilder>(*vm_ctx_);
  auto vm_module = std::make_unique<llvm::Module>(std::string{name}, *vm_ctx_);
  vm_module_ = vm_module.get();
  finalized_ = false;

  auto sys_triple_str = llvm::sys::getProcessTriple();
  auto patched_triple_str = sys_triple_str + "-elf";
  llvm::Triple patched_triple(patched_triple_str);

  llvm::SmallVector<std::string, 4> attrs;
  attrs.push_back("+sse");
  attrs.push_back("+sse2");

  llvm::EngineBuilder engine_builder(std::move(vm_module));
  auto target_machine = engine_builder.selectTarget(patched_triple, "", "", attrs);
  auto engine = engine_builder.setErrorStr(&error_).create(target_machine);

  vm_engine_.reset(engine);
}

llvm::Module *module_vmcode_impl::get_vm_module() const { return vm_module_; }

llvm::LLVMContext &module_vmcode_impl::get_vm_context() { return *vm_ctx_; }

module_vmcode_impl::~module_vmcode_impl() {}

llvm::DefaultIRBuilder *module_vmcode_impl::builder() const { return ir_builder_.get(); }

void module_vmcode_impl::dump_ir() const {
  // vm_module_->dump();
}

void module_vmcode_impl::dump_ir(std::ostream &ostr) const {
  llvm::raw_os_ostream raw_os(ostr);
  vm_module_->print(raw_os, nullptr);
  raw_os.flush();
}

module_semantic *module_vmcode_impl::get_semantic() const { return sem_.get(); }

void module_vmcode_impl::set_semantic(shared_ptr<module_semantic> const &v) { sem_ = v; }

module_context *module_vmcode_impl::get_context() const { return ctxt_.get(); }

void module_vmcode_impl::set_context(shared_ptr<module_context> const &v) { ctxt_ = v; }

void *module_vmcode_impl::get_function(string_view func_name) {
  if (!finalized_) {
    vm_engine_->finalizeObject();
    finalized_ = true;
  }

  llvm::Function *vm_func = vm_module_->getFunction(std::string{func_name});
  if (!vm_func) {
    return nullptr;
  }

  void *native_func = vm_engine_->getPointerToFunction(vm_func);
  return native_func;
}

void module_vmcode_impl::inject_function(void *pfn, string_view name) {
  llvm::sys::DynamicLibrary::AddSymbol(std::string{name}, pfn);
  return;
}

} // namespace sasl::codegen
