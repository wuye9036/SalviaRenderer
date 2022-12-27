#pragma once

#include <eflib/utility/shared_declaration.h>
#include <sasl/semantic/semantic_forward.h>

#include <memory>
#include <string_view>

namespace sasl::common {
class diag_chat;
}
namespace sasl::syntax_tree {
EFLIB_DECLARE_STRUCT_SHARED_PTR(node);
} // namespace sasl::syntax_tree

namespace sasl::semantic {

class symbol;
EFLIB_DECLARE_CLASS_SHARED_PTR(module_semantic);
EFLIB_DECLARE_CLASS_SHARED_PTR(reflection_impl);

reflection_impl_ptr reflect(module_semantic_ptr const &sem, sasl::common::diag_chat *diags);

reflection_impl_ptr reflect(module_semantic_ptr const &sem, std::string_view entry_name,
                            sasl::common::diag_chat *diags);

} // namespace sasl::semantic
