#include <sasl/semantic/symbol.h>

#include <sasl/enums/traits.h>
#include <sasl/semantic/caster.h>
#include <sasl/semantic/semantic_diags.h>
#include <sasl/semantic/semantics.h>
#include <sasl/semantic/type_checker.h>
#include <sasl/syntax_tree/declaration.h>
#include <sasl/syntax_tree/expression.h>
#include <sasl/syntax_tree/node.h>

#include <eflib/diagnostics/assert.h>
#include <eflib/utility/polymorphic_cast.h>
#include <eflib/utility/unref_declarator.h>

#include <boost/lexical_cast.hpp>

#include <algorithm>
#include <string_view>

using eflib::polymorphic_cast;
using namespace std;

namespace sasl::semantic {

using sasl::common::diag_chat;
using sasl::enums::is_matrix;
using sasl::enums::is_scalar;
using sasl::enums::is_vector;
using sasl::enums::scalar_of;
using sasl::enums::vector_count;
using sasl::enums::vector_size;
using sasl::syntax_tree::expression;
using sasl::syntax_tree::function_def;
using sasl::syntax_tree::function_type;
using sasl::syntax_tree::tynode;

using std::shared_ptr;

symbol* symbol::create_root(module_semantic* owner, node* root_node) {
  return create(owner, nullptr, root_node);
}

symbol*
symbol::create(module_semantic* owner, symbol* parent, node* assoc_node, string_view mangled) {
  assert(owner->get_symbol(assoc_node) == nullptr);
  symbol* ret = new (owner->alloc_symbol()) symbol(owner, parent, nullptr, mangled);
  owner->link_symbol(assoc_node, ret);
  return ret;
}

symbol* symbol::create(module_semantic* owner, symbol* parent, node* assoc_node) {
  assert(owner->get_symbol(assoc_node) == nullptr);
  symbol* ret = new (owner->alloc_symbol()) symbol(owner, parent, nullptr, string_view{});
  owner->link_symbol(assoc_node, ret);
  return ret;
}

symbol::symbol(module_semantic* owner, symbol* parent, node* assoc_node, string_view mangled)
  : owner_(owner)
  , associated_node_(assoc_node)
  , parent_(parent)
  , mangled_name_(mangled)
  , unmangled_name_(mangled) {
}

symbol* symbol::find_this(string_view mangled) const {
  auto iter = named_children_.find(mangled);
  return iter == named_children_.end() ? nullptr : iter->second;
}

symbol* symbol::find(string_view mangled) const {
  symbol* ret = find_this(mangled);
  if (ret) {
    return ret;
  }
  if (!parent_) {
    return nullptr;
  }
  return parent_->find(mangled);
}

symbol::symbol_array symbol::find_overloads(string_view unmangled) const {
  auto it = overloads_.find(unmangled);
  if (it != overloads_.end()) {
    return it->second.first;
  }
  if (!parent_) {
    return symbol_array();
  }
  return parent_->find_overloads(unmangled);
}

symbol::symbol_array
symbol::find_overloads(string_view unmangled, caster_t* conv, expression_array const& args) const {
  // find all overloads_
  symbol_array overloads = find_overloads_impl(unmangled, conv, args);
  collapse_vector1_overloads(overloads);
  return overloads;
}

symbol::symbol_array symbol::find_assign_overloads(string_view unmangled,
                                                   caster_t* conv,
                                                   expression_array const& args) const {
  symbol_array candidates = find_overloads_impl(unmangled, conv, args);
  tid_t lhs_arg_tid = owner_->get_semantic(args.back())->tid();
  symbol_array ret;
  for (symbol* proto : candidates) {
    function_def* proto_fn = dynamic_cast<function_def*>(proto->associated_node());
    tid_t lhs_par_tid = owner_->get_semantic(proto_fn->params.back())->tid();
    if (lhs_par_tid == lhs_arg_tid) {
      ret.push_back(proto);
    }
  }
  return ret;
}

symbol* symbol::add_named_child(string_view mangled, node* child_node) {
  auto iter = named_children_.find(mangled);
  if (iter != named_children_.end()) {
    return nullptr;
  }

  symbol* ret = add_child(child_node);
  ret->mangled_name_ = ret->unmangled_name_ = mangled;
  named_children_.insert(make_pair(mangled, ret));

  return ret;
}

symbol* symbol::add_function_begin(function_def* child_fn) {
  if (!child_fn) {
    return nullptr;
  }
  auto* ret = new (owner_->alloc_symbol()) symbol(owner_, this, child_fn, string_view{});
  ret->unmangled_name_ = child_fn->name.lit();
  return ret;
}

bool symbol::add_function_end(symbol* sym, tid_t fn_tid) {
  ef_verify(sym != nullptr);
  node* sym_node = sym->associated_node();
  ef_verify(sym_node != nullptr);

  auto iter = overloads_.find(sym->unmangled_name_);
  if (iter == overloads_.end()) {
    iter =
        overloads_
            .insert(make_pair(sym->unmangled_name_, make_pair(vector<symbol*>(), vector<tid_t>())))
            .first;
  }

  // Check function had same signature yet. It is not overloading, it's error.
  auto tid_iter = std::find(iter->second.second.begin(), iter->second.second.end(), fn_tid);
  if (tid_iter != iter->second.second.end()) {
    EFLIB_ASSERT_UNIMPLEMENTED0("Overloads are same signature.");
    return false;
  }

  iter->second.first.push_back(sym);
  iter->second.second.push_back(fn_tid);
  children_.insert(make_pair(sym_node, sym));

  // If associated node of symbol is not null,
  // the old node will remove from symbol-node dictionary firstly.
  // In normal case, we assume that associated node of symbol has not been added to dictionary yet.
  // So we set associated node to nullptr to prevent old node erasing.
  sym->associated_node(nullptr);
  owner_->link_symbol(sym_node, sym);
  return true;
}

void symbol::cancel_function(symbol* /*sym*/) {
  // Do nothing while function is cancelled.
}

void symbol::remove_child(symbol* /*sym*/) {
  ef_unimplemented();
}

void symbol::remove() {
  if (parent_) {
    parent_->remove_child(this);
  }
}

symbol* symbol::parent() const {
  return parent_;
}

node* symbol::associated_node() const {
  return associated_node_;
}

void symbol::associated_node(node* v) {
  associated_node_ = v;
}

string_view symbol::mangled_name() const {
  if (mangled_name_.empty()) {
    // TODO Unavailable until function type is finished.
    node_semantic* node_sem = owner_->get_semantic(associated_node_);
    EFLIB_UNREF_DECLARATOR(node_sem);
    if (associated_node_->node_class() == node_ids::function_def) {
      node* fn_ty = static_cast<function_def*>(associated_node_)->type.get();
      const_cast<symbol*>(this)->mangled_name_ =
          owner_->pety()->mangle(unmangled_name_, owner_->get_semantic(fn_ty)->tid());
    }
  }
  return mangled_name_;
}

string_view symbol::unmangled_name() const {
  return unmangled_name_;
}

symbol* symbol::add_child(node* child_node) {
  auto iter = children_.find(child_node);
  if (iter != children_.end()) {
    return nullptr;
  }

  symbol* ret = create(owner_, this, child_node);
  children_.insert(make_pair(child_node, ret));

  return ret;
}

bool is_equiva(builtin_types bt0, builtin_types bt1) {
  if (!(is_scalar(bt0) || is_vector(bt0) || is_matrix(bt0))) {
    return false;
  }

  if (!(is_scalar(bt1) || is_vector(bt1) || is_matrix(bt1))) {
    return false;
  }

  if (is_scalar(bt0) && is_scalar(bt1)) {
    return false;
  }

  if (is_scalar(bt0) && is_vector(bt1)) {
    return (scalar_of(bt1) == bt0 && vector_size(bt1) == 1);
  }

  if (is_scalar(bt0) && is_matrix(bt1)) {
    return (scalar_of(bt1) == bt0 && vector_size(bt1) == 1 && vector_count(bt1) == 1);
  }

  if (is_vector(bt0) && is_vector(bt1)) {
    return false;
  }

  if (is_vector(bt0) && is_matrix(bt1)) {
    return (scalar_of(bt1) == scalar_of(bt0) && vector_size(bt1) == vector_size(bt0) &&
            vector_count(bt1) == 1);
  }

  if (is_matrix(bt0) && is_matrix(bt1)) {
    return false;
  }

  return is_equiva(bt1, bt0);
}

void is_same_or_equiva(module_semantic* msem, node* nd0, node* nd1, bool& same, bool& equiva) {
  same = false;
  equiva = false;

  node_semantic* nd0_sem = msem->get_semantic(nd0);
  node_semantic* nd1_sem = msem->get_semantic(nd1);

  bool same_tid = (nd0_sem->tid() == nd1_sem->tid());

  if (same_tid) {
    same = true;
    equiva = true;
    return;
  }

  builtin_types nd0_bt = nd0_sem->ty_proto()->tycode;
  builtin_types nd1_bt = nd1_sem->ty_proto()->tycode;
  equiva = is_equiva(nd0_bt, nd1_bt);
}

void symbol::collapse_vector1_overloads(symbol_array& candidates) const {
  symbol_array ret;

  for (symbol* cand : candidates) {
    shared_ptr<function_def> cand_fn = cand->associated_node()->as_handle<function_def>();

    bool matched = false;
    for (symbol* filterated : ret) {
      shared_ptr<function_def> filterated_fn =
          filterated->associated_node()->as_handle<function_def>();
      size_t param_count = filterated_fn->params.size();

      bool same_function = true;
      for (size_t i_param = 0; i_param < param_count; ++i_param) {
        bool same = false;
        bool equiva = false;
        is_same_or_equiva(owner_,
                          cand_fn->params[i_param].get(),
                          filterated_fn->params[i_param].get(),
                          same,
                          equiva);
        if (!(same || equiva)) {
          same_function = false;
          break;
        }
      }
      if (same_function) {
        matched = true;
        break;
      }
    }

    if (!matched) {
      ret.push_back(cand);
    }
  }

  return std::swap(candidates, ret);
}

bool get_deprecated_and_next(symbol* const& sym,
                             symbol* const* begin_addr,
                             vector<bool> const& deprecated) {
  auto i = (intptr_t)std::distance(begin_addr, boost::addressof(sym));
  return !deprecated[i];
}

symbol::symbol_array symbol::find_overloads_impl(string_view unmangled,
                                                 caster_t* conv,
                                                 expression_array const& args) const {
  // find all overloads
  symbol_array overloads = find_overloads(unmangled);
  if (overloads.empty()) {
    return overloads;
  }

  // Extract type info of args
  vector<tid_t> arg_tids;
  vector<node_semantic*> arg_sems;
  for (expression* arg : args) {
    arg_sems.push_back(owner_->get_semantic(arg));
    if (!arg_sems.back()) {
      overloads.clear();
      return overloads;
    }
    arg_tids.push_back(arg_sems.back()->tid());
  }

  // Find candidates.
  // Following steps could impl function overloading :
  //
  //  for each candidate in overloads_
  //    if candidate is a valid overload
  //    add to candidates
  //  now the candidates is result.
  //
  // better & worse judgment is as same as C#.
  symbol_array candidates;
  for (auto& overload : overloads) {
    auto matching_func = overload->associated_node()->as_handle<function_def>();
    // not a match.
    if (matching_func->params.size() != args.size()) {
      continue;
    }

    // try to match all parameters.
    bool all_parameter_success = true;
    for (size_t i_param = 0; i_param < args.size(); ++i_param) {
      node_semantic* par_sem = owner_->get_semantic(matching_func->params[i_param]);
      tid_t arg_type = arg_tids[i_param];
      tid_t par_type = par_sem->tid();
      if (arg_type == -1 || par_type == -1) {
        all_parameter_success = false;
        break;
      }
      if (!(arg_type == par_type || conv->try_implicit(par_type, arg_type))) {
        all_parameter_success = false;
        break;
      }
    }
    if (all_parameter_success) {
      candidates.push_back(overload);
    }
  }

  //  for each candidate in candidates
  //    for each evaluated in candidate successors.
  //      if candidate is better than evaluated, deprecate evaluated.
  //      if candidate is worse than evaluated, deprecated candidate.
  vector<bool> deprecated(candidates.size(), false);
  for (size_t i_cand = 0; i_cand < candidates.size(); ++i_cand) {
    if (deprecated[i_cand]) {
      continue;
    }
    shared_ptr<function_def> a_matched_func =
        (candidates[i_cand])->associated_node()->as_handle<function_def>();
    for (size_t j_cand = i_cand + 1; j_cand < candidates.size(); ++j_cand) {
      if (deprecated[j_cand]) {
        continue;
      }

      shared_ptr<function_def> matching_func =
          (candidates[j_cand])->associated_node()->as_handle<function_def>();

      size_t better_param_count = 0;
      size_t worse_param_count = 0;

      for (size_t i_param = 0; i_param < args.size(); ++i_param) {
        tid_t arg_type = owner_->get_semantic(args[i_param])->tid();
        tid_t matching_par_type = owner_->get_semantic(matching_func->params[i_param])->tid();
        tid_t matched_par_type = owner_->get_semantic(a_matched_func->params[i_param])->tid();

        bool par_is_better = false;
        bool par_is_worse = false;
        conv->better_or_worse(
            matched_par_type, matching_par_type, arg_type, par_is_better, par_is_worse);
        if (par_is_better) {
          ++better_param_count;
        }
        if (par_is_worse) {
          ++worse_param_count;
        }
      }

      if (better_param_count > 0 && worse_param_count == 0) {
        deprecated[i_cand] = true;
      }
      if (better_param_count == 0 && worse_param_count > 0) {
        deprecated[j_cand] = true;
      }
    }
  }

  // Gather candidates.
  if (!candidates.empty()) {
    auto partition_fn = [&candidates, &deprecated](symbol* const& sym) {
      return get_deprecated_and_next(sym, boost::addressof(candidates[0]), deprecated);
    };
    auto it = partition(candidates.begin(), candidates.end(), partition_fn);

    symbol_array ret(candidates.begin(), it);
    candidates.resize(distance(candidates.begin(), it));
  }

  return candidates;
}

symbol::overload_position symbol::get_overload_position(std::string_view name) {
  auto iter = overloads_.find(name);
  if (iter != overloads_.end()) {
    return &(iter->second);
  } else {
    pair<overload_dict::iterator, bool> result =
        overloads_.insert(make_pair(name, make_pair(symbol_array(), vector<tid_t>())));

    return &(result.first->second);
  }
}

symbol*
symbol::unchecked_insert_overload(symbol::overload_position pos, function_def* def, tid_t tid) {
  symbol* sym = symbol::create(owner_, this, def);
  sym->unmangled_name_ = def->name.lit();
  children_.insert(make_pair(static_cast<node*>(def), sym));

  pos.pos->first.push_back(sym);
  pos.pos->second.push_back(tid);

  return sym;
}

module_semantic* symbol::owner() const {
  return owner_;
}

}  // namespace sasl::semantic
