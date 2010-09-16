#include <sasl/include/code_generator/llvm/cgllvm_contexts.h>

BEGIN_NS_SASL_CODE_GENERATOR();

cgllvm_common_context::cgllvm_common_context()
	: llvm_value(NULL),
	llvm_function(NULL),
	llvm_type(NULL)
{
}

END_NS_SASL_CODE_GENERATOR();