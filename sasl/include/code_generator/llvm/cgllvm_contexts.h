#ifndef SASL_CODE_GENERATOR_LLVM_CGLLVM_CONTEXTS_H
#define SASL_CODE_GENERATOR_LLVM_CGLLVM_CONTEXTS_H

#include <sasl/include/code_generator/forward.h>
#include <sasl/include/code_generator/codegen_context.h>

namespace llvm{
	// Node
	class Argument;
	class Function;
	class GlobalVariable;
	class Value;
	
	// Type
	class FunctionType;
	class Type;
}

BEGIN_NS_SASL_CODE_GENERATOR();

//////////////////////////////////////////////////////////
// used by a lot of node for common information storage

class cgllvm_common_context: public codegen_context{
public:
	typedef codegen_context base_type;
	cgllvm_common_context();

	llvm::Value* val;
	llvm::Function* func;
	llvm::Argument* arg;

	const llvm::Type* type;
	// for integral only
	bool is_signed;

	const llvm::FunctionType* func_type;

	llvm::GlobalVariable* gvar;
};

class cgllvm_type_context: public codegen_context{
public:
	typedef codegen_context base_type;
	cgllvm_type_context();

	const llvm::Type* basic_type;
};
END_NS_SASL_CODE_GENERATOR();

#endif