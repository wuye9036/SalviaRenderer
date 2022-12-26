#include <sasl/syntax_tree/utility.h>

#include <sasl/syntax_tree/declaration.h>
#include <sasl/syntax_tree/expression.h>
#include <sasl/syntax_tree/node_creation.h>
#include <sasl/syntax_tree/program.h>
#include <sasl/syntax_tree/statement.h>
#include <sasl/syntax_tree/visitor.h>

#include <eflib/diagnostics/assert.h>
#include <eflib/utility/enable_if.h>

#include <boost/preprocessor.hpp>

#include <any>
#include <memory>
#include <type_traits>
#include <variant>
#include <vector>

using std::vector;

using std::any;
using std::any_cast;
using std::dynamic_pointer_cast;
using std::is_base_of;
using std::shared_ptr;

namespace sasl::syntax_tree {

class follow_up_visitor : public syntax_tree_visitor {
public:
  follow_up_visitor(std::function<void(node &, ::std::any *)> applied) : applied(applied) {}

  template <typename NodePtr, typename = decltype(std::declval<NodePtr>().get())>
  void invoke_accept(NodePtr &&pnode, std::any *data) {
    if (std::forward<NodePtr>(pnode)) {
      pnode->accept(this, data);
    }
  }

  template <typename T> void visit(vector<T> &cont, ::std::any *data) {
    for (auto &e : cont) {
      invoke_accept(e, data);
    }
  }

  // expression
  SASL_VISIT_DCL(unary_expression) {
    invoke_accept(v.expr, data);
    applied(v, data);
  }
  SASL_VISIT_DCL(cast_expression) {
    invoke_accept(v.casted_type, data);
    invoke_accept(v.expr, data);
    applied(v, data);
  }
  SASL_VISIT_DCL(binary_expression) {
    invoke_accept(v.left_expr, data);
    invoke_accept(v.right_expr, data);
    applied(v, data);
  }
  SASL_VISIT_DCL(expression_list) {
    visit(v.exprs, data);
    applied(v, data);
  }
  SASL_VISIT_DCL(cond_expression) {
    invoke_accept(v.cond_expr, data);
    invoke_accept(v.yes_expr, data);
    invoke_accept(v.no_expr, data);
    applied(v, data);
  }
  SASL_VISIT_DCL(index_expression) {
    invoke_accept(v.expr, data);
    invoke_accept(v.index_expr, data);
    applied(v, data);
  }
  SASL_VISIT_DCL(call_expression) {
    invoke_accept(v.expr, data);
    visit(v.args, data);
    applied(v, data);
  }
  SASL_VISIT_DCL(member_expression) {
    invoke_accept(v.expr, data);
    applied(v, data);
  }
  SASL_VISIT_DCL(constant_expression) { applied(v, data); }
  SASL_VISIT_DCL(variable_expression) { applied(v, data); }

  // declaration & type specifier
  SASL_VISIT_INLINE_DEF_UNIMPL(initializer);

  SASL_VISIT_DCL(expression_initializer) {
    invoke_accept(v.init_expr, data);
    applied(v, data);
  }
  SASL_VISIT_DCL(member_initializer) {
    visit(v.sub_inits, data);
    applied(v, data);
  }
  SASL_VISIT_INLINE_DEF_UNIMPL(declaration);
  SASL_VISIT_DCL(variable_declaration) {
    invoke_accept(v.type_info, data);
    visit(v.declarators, data);
    applied(v, data);
  }
  SASL_VISIT_DCL(declarator) {
    invoke_accept(v.init, data);
    applied(v, data);
  }
  SASL_VISIT_DCL(type_definition) {
    invoke_accept(v.type_info, data);
    applied(v, data);
  }
  SASL_VISIT_INLINE_DEF_UNIMPL(tynode);
  SASL_VISIT_DCL(builtin_type) { applied(v, data); }
  SASL_VISIT_DCL(array_type) {
    invoke_accept(v.elem_type, data);
    visit(v.array_lens, data);
    applied(v, data);
  }
  SASL_VISIT_DCL(struct_type) {
    visit(v.decls, data);
    applied(v, data);
  }
  SASL_VISIT_DCL(alias_type) { v.accept(this, data); }
  SASL_VISIT_DCL(function_type) {
    invoke_accept(v.result_type, data);
    visit(v.param_types, data);
    applied(v, data);
  }
  SASL_VISIT_DCL(parameter) {
    invoke_accept(v.init, data);
    applied(v, data);
  }
  SASL_VISIT_DCL(function_def) {
    invoke_accept(v.type, data);
    visit(v.params, data);
    invoke_accept(v.body, data);
    applied(v, data);
  }
  SASL_VISIT_DCL(parameter_full) {
    invoke_accept(v.init, data);
    invoke_accept(v.param_type, data);
    applied(v, data);
  }
  SASL_VISIT_DCL(function_full_def) {
    invoke_accept(v.retval_type, data);
    visit(v.params, data);
    invoke_accept(v.body, data);
    applied(v, data);
  }

  // statement
  SASL_VISIT_INLINE_DEF_UNIMPL(statement);

  SASL_VISIT_DCL(declaration_statement) {
    visit(v.decls, data);
    applied(v, data);
  }
  SASL_VISIT_DCL(if_statement) {
    invoke_accept(v.cond, data);
    invoke_accept(v.yes_stmt, data);
    invoke_accept(v.no_stmt, data);
    applied(v, data);
  }
  SASL_VISIT_DCL(while_statement) {
    invoke_accept(v.cond, data);
    invoke_accept(v.body, data);
    applied(v, data);
  }
  SASL_VISIT_DCL(dowhile_statement) {
    invoke_accept(v.body, data);
    invoke_accept(v.cond, data);
    applied(v, data);
  }
  SASL_VISIT_DCL(for_statement) {
    invoke_accept(v.init, data);
    invoke_accept(v.cond, data);
    invoke_accept(v.iter, data);
    invoke_accept(v.body, data);
    applied(v, data);
  }
  SASL_VISIT_DCL(case_label) {
    invoke_accept(v.expr, data);
    applied(v, data);
  }
  SASL_VISIT_DCL(ident_label) { applied(v, data); }
  SASL_VISIT_DCL(switch_statement) {
    invoke_accept(v.cond, data);
    invoke_accept(v.stmts, data);
    applied(v, data);
  }
  SASL_VISIT_DCL(compound_statement) {
    visit(v.stmts, data);
    applied(v, data);
  }
  SASL_VISIT_DCL(expression_statement) {
    invoke_accept(v.expr, data);
    applied(v, data);
  }
  SASL_VISIT_DCL(jump_statement) {
    invoke_accept(v.jump_expr, data);
    applied(v, data);
  }
  SASL_VISIT_DCL(labeled_statement) {
    invoke_accept(v.stmt, data);
    visit(v.labels, data);
  }
  // program
  SASL_VISIT_DCL(program) {
    visit(v.decls, data);
    applied(v, data);
  }

private:
  std::function<void(node &, ::std::any *)> applied;
};

void follow_up_traversal(std::shared_ptr<node> root,
                         std::function<void(node &, ::std::any *)> on_visit) {
  follow_up_visitor fuv(on_visit);
  if (root) {
    root->accept(&fuv, nullptr);
  }
}

std::shared_ptr<builtin_type> create_builtin_type(const builtin_types &btc) {
  auto ret = create_node<builtin_type>(token::make_empty(), token::make_empty());
  ret->tycode = btc;
  return ret;
}

template <typename NodeT> void store_node_to_data(any *lhs, shared_ptr<NodeT> rhs) {
  if (rhs) {
    *lhs = dynamic_pointer_cast<node>(rhs);
  } else {
    *lhs = shared_ptr<node>();
  }
}

template <typename T>
  requires std::is_copy_assignable_v<T>
void copy_member(T &lhs, T const &rhs) {
  lhs = rhs;
}
template <typename T>
  requires(!std::is_copy_assignable_v<T> /* &&
           std::is_same_v<std::remove_cvref_t<decltype(declval<T>().clone())>, T>*/)
void copy_member(T &lhs, T const &rhs) {
  lhs = rhs.clone();
}

#define COPY_VALUE_ITEM(r, dest_src, member)                                                       \
  copy_member(BOOST_PP_TUPLE_ELEM(2, 0, dest_src)->member,                                         \
              BOOST_PP_TUPLE_ELEM(2, 1, dest_src).member);

#define SASL_SWALLOW_CLONE_NODE(output, v, node_type, member_seq)                                  \
  std::shared_ptr<node_type> cloned = create_node<node_type>(v.token_begin(), v.token_end());      \
  BOOST_PP_SEQ_FOR_EACH(COPY_VALUE_ITEM, (cloned, v), member_seq);                                 \
  store_node_to_data((output), cloned);

template <typename T> void copy_from_any(T &lhs, const std::any &rhs) { lhs = any_cast<T>(rhs); }

template <typename NodeT>
void copy_from_any(shared_ptr<NodeT> &lhs, const any &rhs,
                   EFLIB_ENABLE_IF_PRED2(is_base_of, node, NodeT)) {
  shared_ptr<node> any_v = any_cast<shared_ptr<node>>(rhs);
  if (any_v) {
    lhs = dynamic_pointer_cast<NodeT>(any_v);
  } else {
    lhs.reset();
  }
}

#define DEEPCOPY_VALUE_ITEM(r, dest_src, member)                                                   \
  visit(BOOST_PP_TUPLE_ELEM(2, 1, dest_src).member, &member_dup);                                  \
  copy_from_any(BOOST_PP_TUPLE_ELEM(2, 0, dest_src)->member, member_dup);

#define SASL_DEEP_CLONE_NODE(dest_any_ptr, src_v_ref, node_type, member_seq)                       \
  std::shared_ptr<node_type> cloned =                                                              \
      create_node<node_type>(src_v_ref.token_begin(), src_v_ref.token_end());                      \
  std::any member_dup;                                                                             \
  BOOST_PP_SEQ_FOR_EACH(DEEPCOPY_VALUE_ITEM, (cloned, src_v_ref), member_seq);                     \
  store_node_to_data((dest_any_ptr), cloned);

#define SASL_CLONE_NODE_FUNCTION_DEF(clone_mode, node_type, member_seq)                            \
  SASL_VISIT_DCL(node_type) {                                                                      \
    EFLIB_ASSERT(data, "Data parameter must not be nullptr, it is used to feedback cloned node."); \
    SASL_##clone_mode##_CLONE_NODE(data, v, node_type, member_seq);                                \
  }

class swallow_duplicator : public syntax_tree_visitor {
public:
  SASL_CLONE_NODE_FUNCTION_DEF(SWALLOW, unary_expression, (op)(expr));

  SASL_CLONE_NODE_FUNCTION_DEF(SWALLOW, cast_expression, (casted_type)(expr));
  SASL_CLONE_NODE_FUNCTION_DEF(SWALLOW, binary_expression, (op)(op_token)(left_expr)(right_expr));
  SASL_VISIT_INLINE_DEF_UNIMPL(expression_list);
  SASL_CLONE_NODE_FUNCTION_DEF(SWALLOW, cond_expression, (cond_expr)(yes_expr)(no_expr));
  SASL_CLONE_NODE_FUNCTION_DEF(SWALLOW, index_expression, (expr)(index_expr));
  SASL_CLONE_NODE_FUNCTION_DEF(SWALLOW, call_expression, (expr)(args));
  SASL_CLONE_NODE_FUNCTION_DEF(SWALLOW, member_expression, (expr)(member));
  SASL_CLONE_NODE_FUNCTION_DEF(SWALLOW, constant_expression, (value_tok)(ctype));
  SASL_CLONE_NODE_FUNCTION_DEF(SWALLOW, variable_expression, (var_name));

  // declaration & type specifier
  SASL_VISIT_INLINE_DEF_UNIMPL(initializer);
  SASL_CLONE_NODE_FUNCTION_DEF(SWALLOW, expression_initializer, (init_expr));
  SASL_VISIT_INLINE_DEF_UNIMPL(member_initializer);
  SASL_VISIT_INLINE_DEF_UNIMPL(declaration);
  SASL_CLONE_NODE_FUNCTION_DEF(SWALLOW, variable_declaration, (type_info)(declarators));
  SASL_CLONE_NODE_FUNCTION_DEF(SWALLOW, parameter_full, (name)(param_type)(init));
  SASL_CLONE_NODE_FUNCTION_DEF(SWALLOW, declarator, (name)(init)(semantic)(semantic_index));
  SASL_CLONE_NODE_FUNCTION_DEF(SWALLOW, function_def, (name)(type)(params)(body));
  SASL_CLONE_NODE_FUNCTION_DEF(SWALLOW, parameter, (name)(init));
  SASL_VISIT_INLINE_DEF_UNIMPL(type_definition);
  SASL_VISIT_INLINE_DEF_UNIMPL(tynode);
  SASL_CLONE_NODE_FUNCTION_DEF(SWALLOW, builtin_type, (tycode)(qual)(tok_beg)(tok_end));
  SASL_CLONE_NODE_FUNCTION_DEF(SWALLOW, array_type, (tycode)(qual)(elem_type)(array_lens));
  SASL_CLONE_NODE_FUNCTION_DEF(SWALLOW, struct_type, (tycode)(qual)(name)(decls));
  SASL_CLONE_NODE_FUNCTION_DEF(SWALLOW, alias_type, (tycode)(qual)(alias));
  SASL_CLONE_NODE_FUNCTION_DEF(SWALLOW, function_type, (tycode)(qual)(param_types)(result_type));
  SASL_CLONE_NODE_FUNCTION_DEF(SWALLOW, function_full_def,
                               (tycode)(qual)(name)(retval_type)(params)(body));
  // statement
  SASL_VISIT_INLINE_DEF_UNIMPL(statement);

  SASL_CLONE_NODE_FUNCTION_DEF(SWALLOW, declaration_statement, (decls));
  SASL_CLONE_NODE_FUNCTION_DEF(SWALLOW, if_statement, (cond)(yes_stmt)(no_stmt));
  SASL_CLONE_NODE_FUNCTION_DEF(SWALLOW, while_statement, (cond)(body));
  SASL_CLONE_NODE_FUNCTION_DEF(SWALLOW, dowhile_statement, (cond)(body));
  SASL_CLONE_NODE_FUNCTION_DEF(SWALLOW, for_statement, (init)(cond)(iter)(body));
  SASL_CLONE_NODE_FUNCTION_DEF(SWALLOW, switch_statement, (cond)(stmts));
  SASL_CLONE_NODE_FUNCTION_DEF(SWALLOW, compound_statement, (stmts));
  SASL_CLONE_NODE_FUNCTION_DEF(SWALLOW, expression_statement, (expr));
  SASL_CLONE_NODE_FUNCTION_DEF(SWALLOW, jump_statement, (code)(jump_expr));
  SASL_CLONE_NODE_FUNCTION_DEF(SWALLOW, labeled_statement, (stmt)(labels));
  SASL_CLONE_NODE_FUNCTION_DEF(SWALLOW, case_label, (expr));
  SASL_CLONE_NODE_FUNCTION_DEF(SWALLOW, ident_label, (label_tok));

  // program
  SASL_CLONE_NODE_FUNCTION_DEF(SWALLOW, program, (decls)(name));
};

class deep_duplicator : public syntax_tree_visitor {
public:
  SASL_CLONE_NODE_FUNCTION_DEF(DEEP, unary_expression, (op)(expr)(tok_beg)(tok_end));

  SASL_VISIT_INLINE_DEF_UNIMPL(cast_expression);
  SASL_VISIT_INLINE_DEF_UNIMPL(binary_expression);
  SASL_VISIT_INLINE_DEF_UNIMPL(expression_list);
  SASL_VISIT_INLINE_DEF_UNIMPL(cond_expression);
  SASL_VISIT_INLINE_DEF_UNIMPL(index_expression);
  SASL_VISIT_INLINE_DEF_UNIMPL(call_expression);
  SASL_VISIT_INLINE_DEF_UNIMPL(member_expression);
  SASL_VISIT_INLINE_DEF_UNIMPL(constant_expression);
  SASL_VISIT_INLINE_DEF_UNIMPL(variable_expression);

  // declaration & type specifier
  SASL_VISIT_INLINE_DEF_UNIMPL(initializer);
  SASL_VISIT_INLINE_DEF_UNIMPL(expression_initializer);
  SASL_VISIT_INLINE_DEF_UNIMPL(member_initializer);
  SASL_VISIT_INLINE_DEF_UNIMPL(declaration);
  SASL_VISIT_INLINE_DEF_UNIMPL(variable_declaration);
  SASL_VISIT_INLINE_DEF_UNIMPL(declarator);
  SASL_VISIT_INLINE_DEF_UNIMPL(type_definition);
  SASL_VISIT_INLINE_DEF_UNIMPL(tynode);

  SASL_CLONE_NODE_FUNCTION_DEF(DEEP, builtin_type, (tycode)(qual)(tok_beg)(tok_end));

  SASL_VISIT_INLINE_DEF_UNIMPL(array_type);
  SASL_VISIT_INLINE_DEF_UNIMPL(struct_type);
  SASL_VISIT_INLINE_DEF_UNIMPL(alias_type);
  SASL_VISIT_INLINE_DEF_UNIMPL(parameter_full);
  SASL_VISIT_INLINE_DEF_UNIMPL(function_full_def);
  SASL_VISIT_INLINE_DEF_UNIMPL(parameter);
  SASL_VISIT_INLINE_DEF_UNIMPL(function_def);
  SASL_VISIT_INLINE_DEF_UNIMPL(function_type);
  // statement
  SASL_VISIT_INLINE_DEF_UNIMPL(statement);

  SASL_VISIT_INLINE_DEF_UNIMPL(declaration_statement);
  SASL_VISIT_INLINE_DEF_UNIMPL(if_statement);
  SASL_VISIT_INLINE_DEF_UNIMPL(while_statement);
  SASL_VISIT_INLINE_DEF_UNIMPL(dowhile_statement);
  SASL_VISIT_INLINE_DEF_UNIMPL(for_statement);
  SASL_VISIT_INLINE_DEF_UNIMPL(case_label);
  SASL_VISIT_INLINE_DEF_UNIMPL(ident_label);
  SASL_VISIT_INLINE_DEF_UNIMPL(switch_statement);
  SASL_VISIT_INLINE_DEF_UNIMPL(compound_statement);
  SASL_VISIT_INLINE_DEF_UNIMPL(expression_statement);
  SASL_VISIT_INLINE_DEF_UNIMPL(jump_statement);
  SASL_VISIT_INLINE_DEF_UNIMPL(labeled_statement);

  // program
  SASL_VISIT_INLINE_DEF_UNIMPL(program);

  // If value is "value semantic", copy it as raw data.
  template <typename ValueT> void visit(ValueT &v, std::any *data) { *data = v; }

  // If value is "value semantic", copy it as raw data.
  template <typename NodeT> void visit(vector<shared_ptr<NodeT>> &v, std::any *data) {
    vector<shared_ptr<NodeT>> out_v(v.size());
    for (shared_ptr<NodeT> item : v) {
      std::any cloned;
      visit(item, &cloned);
      out_v.push_back(dynamic_pointer_cast<NodeT>(std::any_cast<shared_ptr<node>>(cloned)));
    }
    *data = out_v;
  }
};

template <typename ValueT> ValueT process_node(std::shared_ptr<node> src, syntax_tree_visitor *v) {
  EFLIB_ASSERT_AND_IF(src && v, "The input parameter is unavaliable!") { return src; }

  std::any result_val;
  src->accept(v, &result_val);
  return std::any_cast<ValueT>(result_val);
}

std::shared_ptr<node> duplicate(std::shared_ptr<node> src) {
  swallow_duplicator dup;
  return process_node<std::shared_ptr<node>>(src, &dup);
}

std::shared_ptr<node> deep_duplicate(std::shared_ptr<node> src) {
  deep_duplicator dup;
  return process_node<std::shared_ptr<node>>(src, &dup);
}

} // namespace sasl::syntax_tree
