#include <sasl/include/code_generator/llvm/cgllvm_contexts.h>

BEGIN_NS_SASL_CODE_GENERATOR();

cgllvm_sctxt::cgllvm_sctxt():
val(NULL), func(NULL), parent_func(NULL), arg(NULL),
type(NULL), is_signed(false),
func_type(NULL), addr(NULL),
block(NULL), return_inst(NULL)
{
}

END_NS_SASL_CODE_GENERATOR();