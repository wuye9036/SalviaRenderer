#include <sasl/include/codegen/module_vmcode_impl.h>

#include <sasl/include/semantic/reflector.h>

#include <eflib/include/platform/disable_warnings.h>
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/IRBuilder.h>
#include <llvm/Support/raw_os_ostream.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <eflib/include/platform/enable_warnings.h>

using sasl::semantic::module_semantic;
using eflib::fixed_string;
using boost::shared_ptr;

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
	EFLIB_ASSERT_UNIMPLEMENTED();
	return false;
}

void* module_vmcode_impl::get_function(fixed_string const& /*name*/)
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return NULL;
}

void module_vmcode_impl::inject_function(void* /*pfn*/, fixed_string const& /*name*/)
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return;
}


END_NS_SASL_CODEGEN();
