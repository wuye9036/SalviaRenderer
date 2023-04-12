#pragma once

#include <memory>

namespace sasl::drivers {
class compiler;
std::shared_ptr<compiler> create_compiler();
}  // namespace sasl::drivers
