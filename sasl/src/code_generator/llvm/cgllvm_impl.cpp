#include <sasl/include/code_generator/llvm/cgllvm_impl.imp.h>


#include <sasl/include/syntax_tree/node.h>

using namespace sasl::syntax_tree;

BEGIN_NS_SASL_CODE_GENERATOR();

sctxt_handle cgllvm_impl::node_ctxt( node& nd, bool create_if_need ){
	return node_ctxt(nd.handle(), create_if_need);
}

END_NS_SASL_CODE_GENERATOR();