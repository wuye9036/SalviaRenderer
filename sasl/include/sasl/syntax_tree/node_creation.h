#ifndef SASL_SYNTAX_TREE_NODE_CREATION_H
#define SASL_SYNTAX_TREE_NODE_CREATION_H

#include <sasl/syntax_tree/syntax_tree_fwd.h>

#include <memory>
#include <type_traits>

namespace sasl::syntax_tree() {

struct node;

template <typename NodeT>
std::shared_ptr<NodeT> create_node()
{
	static_assert(std::is_base_of<node, NodeT>::value);
    auto ret = std::shared_ptr<NodeT>{ new NodeT{} };
	return ret;
}

template <typename NodeT, typename ParamT>
std::shared_ptr<NodeT> create_node(ParamT&& par)
{
	static_assert(std::is_base_of<node, NodeT>::value);
    auto ret = std::shared_ptr<NodeT>{ new NodeT(std::forward<ParamT>(par)) };
	return ret;
}

template <typename NodeT>
std::shared_ptr<NodeT> create_node(
	std::shared_ptr<token_t> const& token_beg,
	std::shared_ptr<token_t> const& token_end
	)
{
	static_assert(std::is_base_of<node, NodeT>::value);
    auto ret = std::shared_ptr<NodeT>{ new NodeT(token_beg, token_end) };
	return ret;
}

}

#endif