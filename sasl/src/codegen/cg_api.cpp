#include <sasl/codegen/cg_api.h>

#include <sasl/codegen/cg_general.h>
#include <sasl/codegen/cg_ps.h>
#include <sasl/codegen/cg_vs.h>

#include <sasl/semantic/reflection_impl.h>
#include <sasl/semantic/semantics.h>
#include <sasl/semantic/symbol.h>
#include <sasl/syntax_tree/node.h>

#include <eflib/diagnostics/assert.h>
#include <eflib/utility/shared_declaration.h>

namespace sasl::codegen {

EFLIB_USING_SHARED_PTR(sasl::semantic, module_semantic);
EFLIB_USING_SHARED_PTR(sasl::semantic, reflection_impl);
EFLIB_USING_SHARED_PTR(sasl::syntax_tree, node);

using sasl::semantic::symbol;
using std::shared_ptr;

module_vmcode_ptr generate_vmcode(module_semantic_ptr const& sem,
                                  reflection_impl const* reflection) {
  module_vmcode_ptr ret;

  symbol* root = sem->root_symbol();
  if (!root) {
    return ret;
  }

  node* assoc_node = root->associated_node();
  if (!assoc_node) {
    return ret;
  }
  if (assoc_node->node_class() != node_ids::program) {
    return ret;
  }

  if (!reflection || reflection->get_language() == salvia::shader::lang_general) {
    cg_general cg;
    if (cg.generate(sem, reflection)) {
      return cg.generated_module();
    }
  }

  if (reflection->get_language() == salvia::shader::lang_vertex_shader) {
    cg_vs cg;
    if (cg.generate(sem, reflection)) {
      return cg.generated_module();
    }
  }

  if (reflection->get_language() == salvia::shader::lang_pixel_shader) {
    cg_ps cg;
    if (cg.generate(sem, reflection)) {
      return cg.generated_module();
    }
  }

  return ret;
}

}  // namespace sasl::codegen
