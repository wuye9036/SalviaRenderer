#pragma once

#include <sasl/semantic/semantic_forward.h>

#include <salvia/shader/constants.h>
#include <eflib/utility/shared_declaration.h>

#include <memory>

namespace sasl {
namespace syntax_tree {
EFLIB_DECLARE_STRUCT_SHARED_PTR(node);
}
} // namespace sasl

namespace salviar {
EFLIB_DECLARE_CLASS_SHARED_PTR(shader_reflection2);
}

namespace sasl::semantic {

class symbol;
EFLIB_DECLARE_CLASS_SHARED_PTR(module_semantic);

salviar::shader_reflection2_ptr reflect2(module_semantic_ptr const &sem);
salviar::shader_reflection2_ptr reflect2(module_semantic_ptr const &sem,
                                         std::string_view entry_name);

} // namespace sasl::semantic