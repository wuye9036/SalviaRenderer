#include <sasl/include/code_generator/llvm/cgllvm_context.h>

BEGIN_NS_SASL_CODE_GENERATOR();

cgllvm_context::cgllvm_context( const std::string& mod_name)
	: lctxt(), mod( new llvm::Module( mod_name, lctxt ) )
{
}

boost::shared_ptr<llvm::Module> cgllvm_context::module() const{
	return mod;
}
END_NS_SASL_CODE_GENERATOR();