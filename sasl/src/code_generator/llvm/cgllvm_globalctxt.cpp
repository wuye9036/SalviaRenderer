#include <sasl/include/code_generator/llvm/cgllvm_globalctxt.h>

BEGIN_NS_SASL_CODE_GENERATOR();

cgllvm_global_context::cgllvm_global_context()
{
	lctxt.reset( new llvm::LLVMContext() );
}

void cgllvm_global_context::create_module( const std::string& modname ){
	mod.reset( new llvm::Module( modname, *lctxt ) );
}

boost::shared_ptr<llvm::Module> cgllvm_global_context::module() const{
	return mod;
}

llvm::LLVMContext& cgllvm_global_context::context(){
	return *lctxt;
}

cgllvm_global_context::~cgllvm_global_context(){
}

END_NS_SASL_CODE_GENERATOR();