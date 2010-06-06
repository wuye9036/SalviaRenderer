#ifndef SASL_CODE_GENERATOR_LLVM_LLVM_SEMANTIC_INFO_H
#define SASL_CODE_GENERATOR_LLVM_LLVM_SEMANTIC_INFO_H

#include <sasl/include/code_generator/forward.h>
#include <sasl/include/semantic/semantic_info.h>

namespace llvm{
	class Value;
	class Function;
	class Type;
	class GlobalVariable;
}

BEGIN_NS_SASL_CODE_GENERATOR();

class llvm_semantic_info: public sasl::semantic::semantic_info{
public:
	typedef sasl::semantic::semantic_info base_type;
	llvm_semantic_info();

	llvm::Value* llvm_value;
	llvm::Function* llvm_function;
	const llvm::Type* llvm_type;
	llvm::GlobalVariable* llvm_gvar;
};

END_NS_SASL_CODE_GENERATOR();

#endif