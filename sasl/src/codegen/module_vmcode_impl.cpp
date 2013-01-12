#include <sasl/include/codegen/module_vmcode_impl.h>

#include <sasl/include/semantic/reflector.h>

#include <eflib/include/platform/cpuinfo.h>

#include <eflib/include/platform/disable_warnings.h>
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/IRBuilder.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/raw_os_ostream.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/JIT.h>
#include <eflib/include/platform/enable_warnings.h>

#include <string>
#include <vector>

using sasl::semantic::module_semantic;
using eflib::fixed_string;
using boost::shared_ptr;
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

BEGIN_NS_SASL_CODEGEN();

module_vmcode_impl::module_vmcode_impl(fixed_string const& name)
: vm_engine_(NULL)
{
	vm_ctx_		= new llvm::LLVMContext();
	irbuilder_	= new llvm::IRBuilder<>(*vm_ctx_);
	vm_module_	= new llvm::Module(name.raw_string(), *vm_ctx_);
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
	if(vm_engine_)
	{
		for(size_t i = 0; i < jitted_funcs_.size(); ++i)
		{
			vm_engine_->freeMachineCodeForFunction(jitted_funcs_[i]);
		}
		delete vm_engine_;
	}
	else
	{
		delete vm_module_;
	}

	delete irbuilder_;
	delete vm_ctx_;
}

llvm::DefaultIRBuilder* module_vmcode_impl::builder() const{
	return irbuilder_;
}

void module_vmcode_impl::dump_ir() const
{
	vm_module_->dump();
}

void module_vmcode_impl::dump_ir( std::ostream& ostr ) const
{
	llvm::raw_os_ostream raw_os(ostr);
	vm_module_->print( raw_os, NULL );
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

bool module_vmcode_impl::enable_jit()
{
	if (vm_engine_)
	{
		return true;
	}
	
	if(!error_.empty())
	{
		return false;
	}
	
	llvm_options::initialize();

	// Add Attrs
	vector<string> attrs;
	if( eflib::support_feature(eflib::cpu_sse2) )
	{
		attrs.push_back("+sse");
		attrs.push_back("+sse2");
	}
	
	llvm::TargetOptions opts;

	std::string err_str;

	vm_engine_ = 
		llvm::EngineBuilder(vm_module_)
		.setTargetOptions(opts)
		.setMAttrs(attrs)
		.setErrorStr(&err_str)
		.create();

	if(vm_engine_ == NULL)
	{
		error_ = err_str;
	}
	else
	{
		error_ = fixed_string();
	}

	return vm_engine_ != NULL;
}

void* module_vmcode_impl::get_function(fixed_string const& func_name)
{
	llvm::Function* vm_func = vm_module_->getFunction( func_name.raw_string() );
	if (!vm_func)
	{
		return NULL;
	}

	void* native_func = vm_engine_->getPointerToFunction(vm_func);
	if( find(jitted_funcs_.begin(), jitted_funcs_.end(), vm_func) == jitted_funcs_.end() )
	{
		jitted_funcs_.push_back(vm_func);
	}

	return native_func;
}

void module_vmcode_impl::inject_function(void* pfn, fixed_string const& name)
{
	assert(vm_engine_);
	if(!vm_engine_)
	{
		return;
	}

	llvm::Function* func = vm_module_->getFunction( name.raw_string() );
	if (func)
	{
		vm_engine_->addGlobalMapping(func, pfn);
	}
	return;
}


END_NS_SASL_CODEGEN();
