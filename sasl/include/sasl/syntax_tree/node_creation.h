#pragma once

#include <sasl/syntax_tree/syntax_tree_fwd.h>

#include <sasl/common/token.h>

#include <memory>
#include <type_traits>

namespace sasl::syntax_tree {

struct node;
using token = sasl::common::token;

template <typename NodeT>
  requires std::is_base_of_v<node, NodeT>
std::shared_ptr<NodeT> create_node() {
  return std::make_shared<NodeT>();
}

template <typename NodeT, typename ParamT>
  requires std::is_base_of_v<node, NodeT>
std::shared_ptr<NodeT> create_node(ParamT&& par) {
  return std::make_shared<NodeT>(std::forward<ParamT>(par));
}

template <typename NodeT>
  requires std::is_base_of_v<node, NodeT>
std::shared_ptr<NodeT> create_node(token tok_beg, token tok_end) {
  auto ret = std::make_shared<NodeT>();
  ret->tok_beg = std::move(tok_beg);
  ret->tok_end = std::move(tok_end);
  return ret;
}

}  // namespace sasl::syntax_tree