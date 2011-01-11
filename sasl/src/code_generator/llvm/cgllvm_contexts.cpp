#include <sasl/include/code_generator/llvm/cgllvm_contexts.h>

BEGIN_NS_SASL_CODE_GENERATOR();

cgllvm_common_context::cgllvm_common_context():
val(NULL), func(NULL), parent_func(NULL), arg(NULL),
type(NULL), is_signed(false),
func_type(NULL), addr(NULL),
block(NULL), return_inst(NULL)
{
}

// llvm context for type specifier
cgllvm_type_context::cgllvm_type_context()
	:basic_type(NULL){
}

END_NS_SASL_CODE_GENERATOR();