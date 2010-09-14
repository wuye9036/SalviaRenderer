#include <sasl/include/code_generator/llvm/cgllvm_context.h>

BEGIN_NS_SASL_CODE_GENERATOR();

cgllvm_context::cgllvm_context( const std::string& mod_name)
{
	lctxt.reset( new llvm::LLVMContext() );
	mod.reset( new llvm::Module( mod_name, context() ) );
}

boost::shared_ptr<llvm::Module> cgllvm_context::module() const{
	return mod;
}

llvm::LLVMContext& cgllvm_context::context(){
	return *lctxt;
}

cgllvm_context::~cgllvm_context(){
}

END_NS_SASL_CODE_GENERATOR();