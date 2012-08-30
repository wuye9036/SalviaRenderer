#include <sasl/include/codegen/cg_impl.h>

#include <sasl/include/codegen/cg_contexts.h>
#include <sasl/include/syntax_tree/node.h>

using sasl::syntax_tree::node;
using boost::shared_ptr;

BEGIN_NS_SASL_CODEGEN();

template <typename NodeT>
void cg_impl::visit_child(shared_ptr<NodeT> const& child)
{
	child->accept(this, NULL);
}

template<typename NodeT>
node_context* cg_impl::node_ctxt( shared_ptr<NodeT> const& nd, bool create_if_need ){
	if ( !nd ){ return NULL; }
	node* ptr = static_cast<node*>(nd.get());
	return node_ctxt( ptr, create_if_need );
}

template <typename NodeT>
node_context* cg_impl::node_ctxt(
	NodeT const& nd,
	bool create_if_need /*= false */,
	typename boost::disable_if< std::is_pointer<NodeT> >::type* /*dummy = NULL*/
	)
{
	return node_ctxt( static_cast<node const*>(&nd), create_if_need );
}
END_NS_SASL_CODEGEN();
