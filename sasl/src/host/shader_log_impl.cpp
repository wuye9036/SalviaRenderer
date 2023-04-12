#include <sasl/host/shader_log_impl.h>

namespace sasl::host() {

size_t shader_log_impl::count() const {
  return logs_.size();
}

std::string const& shader_log_impl::log_string(size_t index) const {
  return logs_[index];
}

void shader_log_impl::append(std::string const& str) {
  logs_.push_back(str);
}

}  // namespace sasl::host()