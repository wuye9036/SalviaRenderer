#include <sasl/include/code_generator/llvm/cg_llvm.h>

BEGIN_NS_SASL_CODE_GENERATOR();

using sasl::syntax_tree::node;

boost::shared_ptr<llvm::Module> generate_llvm_code( boost::shared_ptr<node> root ){
	return boost::shared_ptr<llvm::Module>();
}

END_NS_SASL_CODE_GENERATOR();