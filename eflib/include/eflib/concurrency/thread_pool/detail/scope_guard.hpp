#pragma once

#include <functional>

namespace eflib {
namespace threadpool {
namespace detail {

// TODO documentation
class scope_guard {
  std::function<void()> const m_function;
  bool m_is_active;

public:
  scope_guard(std::function<void()> const& call_on_exit)
    : m_function(call_on_exit)
    , m_is_active(true) {}

  ~scope_guard() {
    if (m_is_active && m_function) {
      m_function();
    }
  }

  void disable() { m_is_active = false; }
};

}  // namespace detail
}  // namespace threadpool
}  // namespace eflib
