#include <sasl/include/code_generator/llvm/cgllvm_globalctxt.h>

BEGIN_NS_SASL_CODE_GENERATOR();

cgllvm_global_context::cgllvm_global_context()
: mod(NULL), have_mod(true)
{
	lctxt.reset( new llvm::LLVMContext() );
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

END_NS_SASL_CODE_GENERATOR();