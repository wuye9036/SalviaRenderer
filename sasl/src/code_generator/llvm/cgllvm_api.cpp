#include <sasl/include/code_generator/llvm/cgllvm_api.h>
#include <sasl/include/code_generator/llvm/cgllvm_impl.h>
#include <sasl/include/syntax_tree/node.h>

BEGIN_NS_SASL_CODE_GENERATOR();

using sasl::syntax_tree::node;

boost::shared_ptr<llvm_code> generate_llvm_code( boost::shared_ptr<node> root ){
	if ( root && root->node_class() == syntax_node_types::program ){
		llvm_code_generator cg;
		root->accept(&cg);
		return cg.generated_module();
	}
	return boost::shared_ptr<llvm_code>();
}

END_NS_SASL_CODE_GENERATOR();