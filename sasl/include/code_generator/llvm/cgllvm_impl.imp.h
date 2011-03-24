#include <sasl/include/code_generator/llvm/cgllvm_impl.h>

using boost::any;
using boost::shared_ptr;

BEGIN_NS_SASL_CODE_GENERATOR();

template <typename NodeT> any& cgllvm_impl::visit_child( any& child_ctxt, const any& init_data, shared_ptr<NodeT> child )
{
	child_ctxt = init_data;
	return visit_child( child_ctxt, child );
}

template <typename NodeT> any& cgllvm_impl::visit_child( any& child_ctxt, shared_ptr<NodeT> child )
{
	child->accept( this, &child_ctxt );
	return child_ctxt;
}

END_NS_SASL_CODE_GENERATOR();