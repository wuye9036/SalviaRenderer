#include <sasl/include/code_generator/llvm/cgllvm_impl.h>
#include <sasl/include/code_generator/llvm/cgllvm_contexts.h>

#include <sasl/include/syntax_tree/node.h>

using sasl::syntax_tree::node;

using boost::any;
using boost::shared_ptr;

BEGIN_NS_SASL_CODE_GENERATOR();

template <typename NodeT> any& cgllvm_impl::visit_child( any& child_ctxt, const any& init_data, shared_ptr<NodeT> const& child )
{
	child_ctxt = init_data;
	return visit_child( child_ctxt, child );
}

template <typename NodeT> any& cgllvm_impl::visit_child( any& child_ctxt, shared_ptr<NodeT> const& child )
{
	child->accept( this, &child_ctxt );
	return child_ctxt;
}

template<typename NodeT, typename ContextT>
ContextT* cgllvm_impl::node_ctxt( shared_ptr<NodeT> const& nd, bool create_if_need ){
	if ( !nd ){ return NULL; }

	node* ptr = static_cast<node*>(nd.get());
	ctxts_t::iterator it = ctxts.find( ptr );
	if ( it == ctxts.end() ){
		if( create_if_need ){
			shared_ptr<ContextT> const& ret
				= ctxts[ptr]
				= create_codegen_context<ContextT>( nd->handle() );
			return ret.get();
		}
		return NULL;
	}

	return (it->second).get();
}

template<typename ContextT>
ContextT* cgllvm_impl::node_ctxt( node& nd, bool create_if_need ){
	return node_ctxt<node, ContextT>(nd.handle(), create_if_need);
}

END_NS_SASL_CODE_GENERATOR();