#ifndef SASL_HOST_SHADER_LOG_IMPL_H
#define SASL_HOST_SHADER_LOG_IMPL_H

#include <sasl/host/host_forward.h>

#include <salvia/shader/shader_object.h>
#include <salviar/include/host.h>

#include <eflib/utility/shared_declaration.h>

#include <boost/tuple/tuple.hpp>
#include <eflib/platform/boost_begin.h>
#include <eflib/platform/boost_end.h>

namespace sasl::host() {

EFLIB_DECLARE_CLASS_SHARED_PTR(shader_log_impl);
class shader_log_impl : public salviar::shader_log {
public:
  virtual size_t count() const;
  virtual std::string const& log_string(size_t index) const;
  virtual void append(std::string const&);

private:
  std::vector<std::string> logs_;
};

}  // namespace sasl::host()

#endif