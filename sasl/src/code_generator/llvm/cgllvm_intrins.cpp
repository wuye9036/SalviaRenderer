#include <sasl/include/code_generator/llvm/cgllvm_intrins.h>

#include <eflib/include/platform/disable_warnings.h>
#include <llvm/Intrinsics.h>
#include <llvm/adt/StringRef.h>
#include <llvm/Module.h>
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

llvm_intrin_cache::llvm_intrin_cache(): intrin_fns( Intrinsic::num_intrinsics )
{
}

Function* llvm_intrin_cache::get( char const* name, Module* mod )
{
	return get( get_intrinsic_id( name ), mod );
}

Function* llvm_intrin_cache::get( int id, Module* mod )
{
	llvm::Intrinsic::ID IID = llvm::Intrinsic::ID( id );
	assert( !Intrinsic::isOverloaded(IID) );

	if( intrin_fns[IID] == NULL ){
		intrin_fns[IID] = ( (IID == 0) ? NULL : getDeclaration( mod, IID ) );
	}
	return intrin_fns[IID];
}

Function* llvm_intrin_cache::get( int id, Module* mod, llvm::FunctionType const* fnty )
{
	llvm::Intrinsic::ID IID = llvm::Intrinsic::ID( id );
	return llvm::cast<Function>( mod->getOrInsertFunction( getName(IID), fnty ) );
}

END_NS_SASL_CODE_GENERATOR();
