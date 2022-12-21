#include <sasl/syntax_tree/node.h>
#include <sasl/syntax_tree/visitor.h>

namespace sasl::syntax_tree {

template <typename DerivedT, node_ids TypeId>
void node_impl<DerivedT, TypeId>::accept(syntax_tree_visitor *vis, std::any *data) {
  vis->visit(static_cast<DerivedT &>(*this, data));
}
} // namespace sasl::syntax_tree