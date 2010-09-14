#include <sasl/include/code_generator/llvm/cgllvm_info.h>

BEGIN_NS_SASL_CODE_GENERATOR();

llvm_semantic_info::llvm_semantic_info()
	: llvm_value(NULL),
	llvm_function(NULL),
	llvm_type(NULL)
{
}

END_NS_SASL_CODE_GENERATOR();