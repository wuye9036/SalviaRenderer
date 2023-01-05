#pragma once

#include <sasl/common/token.h>
#include <sasl/enums/node_ids.h>
#include <sasl/syntax_tree/visitor.h>

#include <eflib/utility/shared_declaration.h>

#include <any>
#include <memory>
#include <vector>

namespace sasl {
namespace semantic {
EFLIB_DECLARE_CLASS_SHARED_PTR(symbol);
EFLIB_DECLARE_CLASS_SHARED_PTR(node_semantic);
} // namespace semantic
namespace codegen {
EFLIB_DECLARE_CLASS_SHARED_PTR(module_vmcode);
}
} // namespace sasl

namespace sasl::syntax_tree {

class syntax_tree_visitor;

EFLIB_DECLARE_STRUCT_SHARED_PTR(node);

struct node : public std::enable_shared_from_this<node> {
protected:
  using token = sasl::common::token;
  token tok_beg, tok_end;
  virtual ~node() = default;

public:
  node() = default;

  template <typename R> friend std::shared_ptr<R> create_node();
  template <typename R, typename P0> friend std::shared_ptr<R> create_node(P0 &&);
  template <typename R>
    requires std::is_base_of_v<node, R>
  friend std::shared_ptr<R> create_node(token, token);

  friend class swallow_duplicator;
  friend class deep_duplicator;

  node_ptr as_handle() const { return const_cast<node *>(this)->shared_from_this(); }
  template <typename T> std::shared_ptr<T> as_handle() const {
    return std::dynamic_pointer_cast<T>(as_handle());
  }

  token const &token_begin() const &noexcept { return tok_beg; }
  token const &token_end() const &noexcept { return tok_end; }

  token &token_begin() &noexcept { return tok_beg; }
  token &token_end() &noexcept { return tok_end; }

  token &&token_begin() &&noexcept { return std::move(tok_beg); }
  token &&token_end() &&noexcept { return std::move(tok_end); }

  void token_range(token tok_beg, token tok_end) {
    this->tok_beg = std::move(tok_beg);
    this->tok_end = std::move(tok_end);
  }

  virtual node_ids node_class() const noexcept = 0;

  virtual void accept(syntax_tree_visitor *vis, std::any *data) = 0;

protected:
  node &operator=(const node &) = delete;
  node(const node &) = delete;
};

template <typename DerivedT, typename BaseT, node_ids TypeId> class node_impl : public BaseT {
public:
  constexpr node_ids node_class() const noexcept override { return TypeId; }
  virtual void accept(syntax_tree_visitor *vis, std::any *data) override {
    vis->visit(static_cast<DerivedT &>(*this), data);
  }
};

// template <typename NodeT> std::shared_ptr<NodeT> create_node();
// template <typename R, typename P0> std::shared_ptr<R> create_node( P0 );
// template <typename R, typename P0, typename P1> std::shared_ptr<R> create_node( P0, P1 );

} // namespace sasl::syntax_tree
