#ifndef SASL_CODE_GENERATOR_LLVM_CGLLVM_CONTEXTS_H
#define SASL_CODE_GENERATOR_LLVM_CGLLVM_CONTEXTS_H

#include <sasl/include/code_generator/forward.h>
#include <sasl/include/code_generator/codegen_context.h>

namespace llvm{
	class Value;
	class Function;
	class Type;
	class GlobalVariable;
}

BEGIN_NS_SASL_CODE_GENERATOR();

//////////////////////////////////////////////////////////
// used by a lot of node for common information storage

class cgllvm_common_context: public codegen_context{
public:
	typedef codegen_context base_type;
	cgllvm_common_context();

	llvm::Value* llvm_value;
	llvm::Function* llvm_function;
	const llvm::Type* llvm_type;
	llvm::GlobalVariable* llvm_gvar;
};

END_NS_SASL_CODE_GENERATOR();

#endif