#ifndef SASL_CODEGEN_CG_INTRINS_H
#define SASL_CODEGEN_CG_INTRINS_H

#include <sasl/include/codegen/forward.h>
#include <vector>

namespace llvm{
	class Function;
	class Module;
	class LLVMContext;
	class FunctionType;
}

namespace sasl::codegen {

class llvm_intrin_cache{
public:
	llvm_intrin_cache();
	llvm::Function* get( char const*, llvm::Module* );
	llvm::Function* get( int, llvm::Module* );
	llvm::Function* get( int, llvm::Module*, llvm::FunctionType* );
private:
	std::vector<llvm::Function*> intrin_fns;
};

}


#endif