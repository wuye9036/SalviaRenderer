#pragma once

#include <sasl/codegen/forward.h>

#include <sasl/semantic/caster.h>

#include <functional>
#include <memory>

namespace sasl {
namespace semantic {
class caster_t;
class pety_t;
class symbol;
} // namespace semantic
namespace syntax_tree {
struct node;
}
} // namespace sasl

namespace llvm {
class IRBuilderBase;
class Value;
} // namespace llvm

namespace sasl::codegen {

struct node_context;
class cg_service;

typedef std::function<node_context *(sasl::syntax_tree::node const *)> get_context_fn;

std::shared_ptr<::sasl::semantic::caster_t>
create_cg_caster(get_context_fn const &get_context,
                 sasl::semantic::get_semantic_fn const &get_semantic,
                 sasl::semantic::get_tynode_fn const &get_tynode, cg_service *cgs);

void add_builtin_casts(std::shared_ptr<::sasl::semantic::caster_t> caster,
                       sasl::semantic::pety_t *typemgr);

} // namespace sasl::codegen
