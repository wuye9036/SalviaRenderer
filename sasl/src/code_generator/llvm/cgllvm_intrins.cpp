#include <sasl/include/code_generator/llvm/cgllvm_intrins.h>

#include <eflib/include/platform/disable_warnings.h>
#include <llvm/Intrinsics.h>
#include <llvm/adt/StringRef.h>
#include <eflib/include/platform/enable_warnings.h>

using llvm::Function;
using llvm::Module;
using llvm::LLVMContext;
using llvm::StringRef;

namespace Intrinsic = llvm::Intrinsic;

BEGIN_NS_SASL_CODE_GENERATOR();

llvm::Intrinsic::ID get_intrinsic_id( char const* Name ){
	unsigned Len = strlen(Name);
	
	if (Len < 5 || Name[4] != '.' || Name[0] != 'l' || Name[1] != 'l'
		|| Name[2] != 'v' || Name[3] != 'm')
		return llvm::Intrinsic::ID(0);  // All intrinsics start with 'llvm.'

#define GET_FUNCTION_RECOGNIZER
#include "llvm/Intrinsics.gen"
#undef GET_FUNCTION_RECOGNIZER

	return llvm::Intrinsic::ID(0);
}

class llvm_intrin_cache_impl{
public:
	llvm_intrin_cache_impl( llvm::Module* mod, llvm::LLVMContext& ctxt ): mod(mod), ctxt(ctxt){
		memset( intrin_functions, 0, sizeof(intrin_functions) );
	}

	Function* get( char const* name );

private:
	llvm::Module* mod;
	llvm::LLVMContext& ctxt;

	Function* intrin_functions[llvm::Intrinsic::num_intrinsics];
};

llvm_intrin_cache::llvm_intrin_cache( Module* mod, LLVMContext& ctxt )
	: impl( new llvm_intrin_cache_impl(mod, ctxt) )
{
}

Function* llvm_intrin_cache::get( char const* name )
{
	return impl->get( name );
}

llvm_intrin_cache::~llvm_intrin_cache()
{

}

Function* llvm_intrin_cache_impl::get( char const* name )
{
	llvm::Intrinsic::ID IID = llvm::Intrinsic::ID( get_intrinsic_id( name ) );
	if( intrin_functions[IID] == NULL ){
		intrin_functions[IID] = ( (IID == 0) ? NULL : getDeclaration( mod, IID ) );
	}
	return intrin_functions[IID];
}

END_NS_SASL_CODE_GENERATOR();
