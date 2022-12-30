#ifndef SASL_SEMANTIC_SEMANTIC_API_H
#define SASL_SEMANTIC_SEMANTIC_API_H

#include <sasl/semantic/semantic_forward.h>

#include <eflib/utility/shared_declaration.h>

#include <eflib/platform/stdint.h>

namespace sasl {
namespace syntax_tree {
EFLIB_DECLARE_STRUCT_SHARED_PTR(node);
}
namespace common {
class diag_chat;
}
} // namespace sasl

namespace sasl::semantic {

EFLIB_DECLARE_CLASS_SHARED_PTR(module_semantic);

module_semantic_ptr analysis_semantic(sasl::syntax_tree::node_ptr const &root,
                                      sasl::common::diag_chat *, uint32_t lang);
module_semantic_ptr analysis_semantic(sasl::syntax_tree::node *root, sasl::common::diag_chat *,
                                      uint32_t lang);

} // namespace sasl::semantic

#endif
