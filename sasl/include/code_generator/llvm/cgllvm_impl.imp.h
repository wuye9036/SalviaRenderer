#include <sasl/include/code_generator/llvm/cgllvm_impl.h>
#include <sasl/include/code_generator/llvm/cgllvm_contexts.h>

#include <sasl/include/syntax_tree/node.h>

using sasl::syntax_tree::node;

using boost::any;
using boost::shared_ptr;

BEGIN_NS_SASL_CODE_GENERATOR();

template <typename NodeT>
void cgllvm_impl::visit_child(shared_ptr<NodeT> const& child)
{
	child->accept(this, NULL);
}

template<typename NodeT>
node_context* cgllvm_impl::node_ctxt( shared_ptr<NodeT> const& nd, bool create_if_need ){
	if ( !nd ){ return NULL; }
	node* ptr = static_cast<node*>(nd.get());
	return node_ctxt( ptr, create_if_need );
}

template <typename NodeT>
node_context* cgllvm_impl::node_ctxt( NodeT const& nd, bool create_if_need /*= false */ )
{
	return node_ctxt( (node*)(&nd), create_if_need );
}

END_NS_SASL_CODE_GENERATOR();