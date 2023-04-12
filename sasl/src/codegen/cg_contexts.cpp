#include <sasl/codegen/cg_contexts.h>
#include <sasl/codegen/cgs.h>

#include <eflib/diagnostics/assert.h>
#include <eflib/utility/unref_declarator.h>

#include <boost/pool/pool.hpp>

using boost::pool;
using sasl::syntax_tree::node;
using std::make_pair;
using std::ostream;
using std::shared_ptr;
using std::unordered_map;
using std::vector;

namespace sasl::codegen {

class module_context_impl : public module_context {
public:
  node_context* get_node_context(node const* v) const override {
    auto it = context_dict_.find(v);
    if (it != context_dict_.end())
      return it->second;
    return nullptr;
  }

  node_context* get_or_create_node_context(node const* v) override {
    node_context* ret = get_node_context(v);
    if (ret) {
      return ret;
    }
    ret = create_temporary_node_context();
    context_dict_.insert(make_pair(v, ret));
    return ret;
  }

  node_context* create_temporary_node_context() override {
    auto* ret = alloc_object<node_context>(contexts_pool_);
    new (ret) node_context(this);
    contexts_.push_back(ret);
    return ret;
  }

  cg_type* create_cg_type() override {
    auto* ret = alloc_object<cg_type>(types_pool_);
    new (ret) cg_type();
    cg_types_.push_back(ret);
    return ret;
  }

  cg_function* create_cg_function() override {
    auto* ret = alloc_object<cg_function>(functions_pool_);
    new (ret) cg_function();
    functions_.push_back(ret);
    return ret;
  }

  module_context_impl()
    : contexts_pool_(sizeof(node_context))
    , types_pool_(sizeof(cg_type))
    , functions_pool_(sizeof(cg_function)) {}

  ~module_context_impl() override {
    deconstruct_objects(contexts_);
    deconstruct_objects(cg_types_);
    deconstruct_objects(functions_);
  }

private:
  template <typename T, typename PoolT>
  static T* alloc_object(PoolT& p) {
    assert(sizeof(T) == p.get_requested_size());
    T* ret = static_cast<T*>(p.malloc());
    return ret;
  }

  template <typename T>
  void deconstruct_object(T* v) {
    v->~T();
  }

  template <typename ContainerT>
  void deconstruct_objects(ContainerT& cont) {
    for (auto it = cont.begin(); it != cont.end(); ++it) {
      deconstruct_object(*it);
    }
  }

  pool<> contexts_pool_;
  pool<> types_pool_;
  pool<> functions_pool_;

  typedef unordered_map<node const*, node_context*> node_context_dict;
  node_context_dict context_dict_;
  vector<node_context*> contexts_;
  vector<cg_type*> cg_types_;
  vector<cg_function*> functions_;
};

shared_ptr<module_context> module_context::create() {
  return shared_ptr<module_context>(new module_context_impl());
}

}  // namespace sasl::codegen
