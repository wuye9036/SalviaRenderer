#ifndef SASL_CODE_GENERATOR_LLVM_LLVM_SYMBOL_INFO_H
#define SASL_CODE_GENERATOR_LLVM_LLVM_SYMBOL_INFO_H

#include <sasl/include/code_generator/forward.h>
#include <sasl/include/semantic/symbol_info.h>

namespace llvm{
	class Value;
	class Function;
	class Type;
}

BEGIN_NS_SASL_CODE_GENERATOR();

class llvm_symbol_info: public sasl::semantic::symbol_info{
public:
	typedef sasl::semantic::symbol_info base_type;
	llvm_symbol_info();

	llvm::Value* llvm_value;
	llvm::Function* llvm_function;
	llvm::Type* llvm_type;
};

END_NS_SASL_CODE_GENERATOR();

#endif