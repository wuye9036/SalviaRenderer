#pragma once
#include <mutex>
#include <atomic>
#include <eflib/platform/config.h>

namespace eflib {
class atomic_mutex {
private:
  std::atomic<bool> state_ {false};

public:
  atomic_mutex() = default;

  bool try_lock() {
    return state_.exchange(true, std::memory_order_acquire) == false;
  }

  void lock() {
    while (!try_lock()) {
      /* busy-wait */
    }
  }

  void unlock() { state_.store(false, std::memory_order_release); }
};

} // namespace eflib
