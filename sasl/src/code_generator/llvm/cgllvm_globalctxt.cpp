#include <sasl/include/code_generator/llvm/cgllvm_globalctxt.h>

#include <eflib/include/platform/disable_warnings.h>
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/Support/IRBuilder.h>
#include <eflib/include/platform/enable_warnings.h>

BEGIN_NS_SASL_CODE_GENERATOR();

cgllvm_global_context::cgllvm_global_context()
: mod(NULL), have_mod(true)
{
	lctxt.reset( new llvm::LLVMContext() );
	irbuilder.reset( new llvm::IRBuilder<>( *lctxt ) );
}

void cgllvm_global_context::create_module( const std::string& modname ){
	mod = new llvm::Module( modname, *lctxt );
	have_mod = true;
}

llvm::Module* cgllvm_global_context::module() const{
	return mod;
}

llvm::LLVMContext& cgllvm_global_context::context(){
	return *lctxt;
}

cgllvm_global_context::~cgllvm_global_context(){
	if( have_mod && mod ){
		delete mod;
		mod = NULL;
		have_mod = false;
	}
}

llvm::Module* cgllvm_global_context::get_ownership() const{
	if ( have_mod ){
		have_mod = false;
		return mod;
	}
	return NULL;
}

boost::shared_ptr<llvm::IRBuilder<> > cgllvm_global_context::builder() const{
	return irbuilder;
}
END_NS_SASL_CODE_GENERATOR();