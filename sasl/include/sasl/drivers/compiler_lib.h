#pragma once

#include <sasl/drivers/drivers_forward.h>

#include <memory>

namespace sasl::drivers {
class compiler;
std::shared_ptr<compiler> create_compiler();
}