#include <sasl/include/codegen/module_vmcode_impl.h>

#include <sasl/include/semantic/reflector.h>

#include <eflib/include/platform/cpuinfo.h>

#include <eflib/include/platform/disable_warnings.h>
#include <llvm/ADT/Triple.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/raw_os_ostream.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/DynamicLibrary.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <eflib/include/platform/enable_warnings.h>

#include <string>
#include <vector>

using sasl::semantic::module_semantic;
using eflib::fixed_string;
using std::shared_ptr;
using std::vector;
using std::string;

struct llvm_options
{
	llvm_options(){
		// Add Options
		char* options[] = {""/*, "-force-align-stack"*/};
		llvm::cl::ParseCommandLineOptions( sizeof(options)/sizeof(char*), options );
	}

	static llvm_options& initialize()
	{
		static llvm_options opt;
		return opt;
	}
};

namespace sasl::codegen {

module_vmcode_impl::module_vmcode_impl(fixed_string const& name)
{
	vm_ctx_		= std::make_unique<llvm::LLVMContext>();
	ir_builder_ = std::make_unique<llvm::DefaultIRBuilder>(*vm_ctx_);
	auto vm_module = std::make_unique<llvm::Module>(name.raw_string(), *vm_ctx_);
	vm_module_ = vm_module.get();
	finalized_ = false;

	std::string err;

	// TODO:
	//   LLVM 3.6.1 doesn't support COFF dynamic loading,
	//   It will cause crash on Windows.
	//   Added "-elf" as hack to resolve this issue.

	auto sys_triple_str = llvm::sys::getProcessTriple();
	auto patched_triple_str = sys_triple_str + "-elf";
	llvm::Triple patched_triple(patched_triple_str);

	llvm::SmallVector<std::string, 4> attrs;
	attrs.push_back("+sse");
	attrs.push_back("+sse2");

	llvm::EngineBuilder engine_builder(std::move(vm_module));
	auto target_machine = engine_builder.selectTarget(patched_triple, "", "", attrs);
	auto engine = engine_builder.setErrorStr(&err).create(target_machine);
	error_ = engine ? fixed_string() : err;

	vm_engine_.reset(engine);
}

llvm::Module* module_vmcode_impl::get_vm_module() const
{
	return vm_module_;
}

llvm::LLVMContext& module_vmcode_impl::get_vm_context()
{
	return *vm_ctx_;
}

module_vmcode_impl::~module_vmcode_impl()
{
}

llvm::DefaultIRBuilder* module_vmcode_impl::builder() const
{
	return ir_builder_.get();
}

void module_vmcode_impl::dump_ir() const
{
    // vm_module_->dump();
}

void module_vmcode_impl::dump_ir( std::ostream& ostr ) const
{
	llvm::raw_os_ostream raw_os(ostr);
	vm_module_->print( raw_os, nullptr );
	raw_os.flush();
}

module_semantic* module_vmcode_impl::get_semantic() const
{
	return sem_.get();
}

void module_vmcode_impl::set_semantic( shared_ptr<module_semantic> const& v )
{
	sem_ = v;
}

module_context* module_vmcode_impl::get_context() const
{
	return ctxt_.get();
}

void module_vmcode_impl::set_context( shared_ptr<module_context> const& v )
{
	ctxt_ = v;
}

void* module_vmcode_impl::get_function(fixed_string const& func_name)
{
	if (!finalized_)
	{
		vm_engine_->finalizeObject();
		finalized_ = true;
	}

	llvm::Function* vm_func = vm_module_->getFunction( func_name.raw_string() );
	if (!vm_func)
	{
		return nullptr;
	}

	void* native_func = vm_engine_->getPointerToFunction(vm_func);
	return native_func;
}

void module_vmcode_impl::inject_function(void* pfn, fixed_string const& name)
{
	llvm::sys::DynamicLibrary::AddSymbol(name.raw_string(), pfn);
	return;
}


}
