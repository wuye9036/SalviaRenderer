#pragma once

#include <sasl/semantic/semantic_forward.h>

#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace sasl {
namespace common {
class diag_chat;
}
namespace syntax_tree {
struct node;
struct expression;
struct function_def;
struct tynode;
}  // namespace syntax_tree
}  // namespace sasl

namespace sasl::semantic {

using sasl::syntax_tree::function_def;
using sasl::syntax_tree::node;
using sasl::syntax_tree::tynode;

class caster_t;
class module_semantic;

typedef int tid_t;

/**
        In sasl, symbol can assigned to any syntax node. But in fact, only some
        sorts of node make senses, such as variable declaration, function and
        struct declaration/definition, type re-definition, statement.

        Every symbol has three names.
        * THE FIRST NAME is "unmangled name", briefly "name". It means
        the literal name of syntax node. For a example, a variable name or a label.
        * THE SECOND NAME is "mangled name". It is useful for function overloading.
        There is a document describing its details.
        * THE THIRD NAME is "full path". It is a compile unit unique name or
        a target unique name, even a global unique name. Some external interface
        may use it.
*/

class symbol {
public:
  typedef std::vector<sasl::syntax_tree::expression*> expression_array;
  typedef std::vector<symbol*> symbol_array;
  typedef symbol_array::iterator symbol_array_iterator;

  class overload_position {
  public:
    friend class symbol;
    overload_position(overload_position const& v) : pos(v.pos) {}

  private:
    typedef std::pair<symbol_array, std::vector<tid_t>> overload_array;
    overload_position(overload_array* pos) : pos(pos) {}
    overload_array* pos;
  };

  static symbol* create_root(module_semantic* owner, node* root_node = nullptr);

  module_semantic* owner() const;

  symbol* find(std::string_view name) const;
  symbol* find_this(std::string_view mangled) const;
  int count(std::string_view name) const;

  symbol_array find_overloads(std::string_view name) const;
  symbol_array
  find_overloads(std::string_view name, caster_t* conv, expression_array const& args) const;
  symbol_array
  find_assign_overloads(std::string_view name, caster_t* conv, expression_array const& args) const;

  symbol* add_named_child(std::string_view mangled, node* child_node);
  symbol* add_child(node* child_node);
  symbol* add_function_begin(function_def* child_fn);
  bool add_function_end(symbol* sym, tid_t fn_tid);
  void cancel_function(symbol* sym);

  void remove_child(symbol*);
  void remove();

  symbol* parent() const;

  node* associated_node() const;
  void associated_node(node*);  ///< Don't call it as common API. It is reserved for internal class.

  std::string_view unmangled_name() const;
  std::string_view mangled_name() const;

  overload_position get_overload_position(std::string_view);
  symbol* unchecked_insert_overload(overload_position pos, function_def* def, tid_t tid);

private:
  static symbol* create(module_semantic* owner, symbol* parent, node* assoc_node);
  static symbol*
  create(module_semantic* owner, symbol* parent, node* assoc_node, std::string_view mangled);

  symbol(module_semantic* owner, symbol* parent, node* assoc_node, std::string_view mangled);

  symbol_array
  find_overloads_impl(std::string_view name, caster_t* conv, expression_array const& args) const;
  void collapse_vector1_overloads(symbol_array& candidates) const;

  typedef std::unordered_map<node*, symbol*> children_dict;
  typedef std::unordered_map<std::string_view, symbol*> named_children_dict;
  typedef std::unordered_map<std::string_view, std::pair<symbol_array, std::vector<tid_t>>>
      overload_dict;

  module_semantic* owner_;
  node* associated_node_;
  symbol* parent_;

  // name
  std::string_view mangled_name_;
  std::string_view unmangled_name_;

  children_dict children_;
  named_children_dict named_children_;
  overload_dict overloads_;
  std::vector<std::string_view> null_overloads_;

  inline static constexpr std::string_view null_name{""};
};

}  // namespace sasl::semantic