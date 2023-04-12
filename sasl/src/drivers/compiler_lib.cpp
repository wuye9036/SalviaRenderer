#include <sasl/drivers/compiler_impl.h>
#include <sasl/drivers/compiler_lib.h>

namespace sasl::drivers {
std::shared_ptr<compiler> create_compiler() {
  return std::make_shared<compiler_impl>();
}
}  // namespace sasl::drivers