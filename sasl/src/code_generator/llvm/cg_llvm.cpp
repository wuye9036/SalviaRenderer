#include <sasl/include/code_generator/llvm/cg_llvm.h>

#include <sasl/include/code_generator/llvm/llvm_generator.h>
#include <sasl/include/syntax_tree/node.h>

BEGIN_NS_SASL_CODE_GENERATOR();

using sasl::syntax_tree::node;

boost::shared_ptr<llvm::Module> generate_llvm_code( boost::shared_ptr<node> root ){
	if ( root && root->node_class() == syntax_node_types::program ){
		llvm_code_generator cg;
		root->accept(&cg);
		return cg.generated_module();
	}
	return boost::shared_ptr<llvm::Module>();
}

END_NS_SASL_CODE_GENERATOR();