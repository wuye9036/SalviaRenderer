#include <sasl/include/code_generator/llvm/llvm_symbol_info.h>

BEGIN_NS_SASL_CODE_GENERATOR();

llvm_symbol_info::llvm_symbol_info()
	: base_type("llvm code generator symbol info"),
	llvm_value(NULL),
	llvm_function(NULL),
	llvm_type(NULL)
{
}

END_NS_SASL_CODE_GENERATOR();