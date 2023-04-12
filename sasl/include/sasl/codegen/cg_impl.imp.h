#include <sasl/codegen/cg_impl.h>

#include <sasl/codegen/cg_contexts.h>
#include <sasl/syntax_tree/node.h>

using sasl::syntax_tree::node;
using std::shared_ptr;

namespace sasl::codegen {

template <typename NodeT>
void cg_impl::visit_child(shared_ptr<NodeT> const& child) {
  child->accept(this, nullptr);
}

template <typename NodeT>
node_context* cg_impl::node_ctxt(shared_ptr<NodeT> const& nd, bool create_if_need) {
  if (!nd) {
    return nullptr;
  }
  node* ptr = static_cast<node*>(nd.get());
  return node_ctxt(ptr, create_if_need);
}

template <typename NodeT>
node_context* cg_impl::node_ctxt(NodeT const& nd,
                                 bool create_if_need,
                                 typename std::enable_if<!std::is_pointer<NodeT>::value>::type*) {
  return node_ctxt(static_cast<node const*>(&nd), create_if_need);
}

}  // namespace sasl::codegen
