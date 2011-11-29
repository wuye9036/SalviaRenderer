#ifndef SASL_CODE_GENERATOR_LLVM_CGLLVM_INTRINS_H
#define SASL_CODE_GENERATOR_LLVM_CGLLVM_INTRINS_H

#include <sasl/include/code_generator/forward.h>
#include <vector>

namespace llvm{
	class Function;
	class Module;
	class LLVMContext;
	class FunctionType;
}

BEGIN_NS_SASL_CODE_GENERATOR();

class llvm_intrin_cache{
public:
	llvm_intrin_cache();
	llvm::Function* get( char const*, llvm::Module* );
	llvm::Function* get( int, llvm::Module* );
	llvm::Function* get( int, llvm::Module*, llvm::FunctionType* );
private:
	std::vector<llvm::Function*> intrin_fns;
};

END_NS_SASL_CODE_GENERATOR();


#endif