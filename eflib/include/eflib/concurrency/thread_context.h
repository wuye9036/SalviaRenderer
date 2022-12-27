#pragma once

#include <eflib/platform/cpuinfo.h>
#include <eflib/platform/typedefs.h>
#include <eflib/utility/extension.h>

#include <range/v3/algorithm/for_each.hpp>
#include <range/v3/range/concepts.hpp>

#include <atomic>

namespace eflib {

struct thread_context {
  class package_cursor {
  private:
    friend struct thread_context;

    thread_context const *owner = nullptr;
    size_t cur = 0;

    package_cursor(thread_context const *owner, size_t cur) : owner(owner), cur(cur) {}

  public:
    package_cursor() = default;
    package_cursor(package_cursor const &rhs) = default;
    package_cursor &operator=(package_cursor const &rhs) = default;

    [[nodiscard]] size_t package_index() const noexcept { return cur; }
    [[nodiscard]] auto item_range() const noexcept {
      size_t beg = cur * owner->package_size;
      size_t end = std::min(beg + owner->package_size, owner->item_count);
      return std::make_pair(beg, end);
    }

    [[nodiscard]] bool valid() const noexcept { return cur < owner->package_count; }
  };

  size_t thread_id;

  size_t item_count;
  std::atomic<size_t> *working_package_id;
  size_t package_size;
  size_t package_count;

  [[nodiscard]] static size_t compute_package_count(size_t item_count, size_t package_size) {
    return (item_count + package_size - 1) / package_size;
  }

  [[nodiscard]] package_cursor next_package() const {
    return package_cursor{this, (*working_package_id)++};
  }
};

template <typename ContextRng>
inline void init_thread_context(ContextRng &&rng, size_t item_count,
                                std::atomic<size_t> *working_package_id, size_t package_size) {
  size_t package_count = thread_context::compute_package_count(item_count, package_size);

  for (size_t i = 0; i < rng.size(); ++i) {
    rng[i] = thread_context{.thread_id = i,
                            .item_count = item_count,
                            .working_package_id = working_package_id,
                            .package_size = package_size,
                            .package_count = package_count};
  }
}

template <typename ThreadFunc>
concept thread_func = std::invocable<ThreadFunc, thread_context const *>;

template <typename ThreadPool>
inline void execute_threads(ThreadPool &&pool, thread_func auto &&fn, size_t item_count,
                            size_t package_size, size_t thread_count = eflib::num_available_threads()) {
  // Compute package information
  std::atomic<size_t> working_package(0);
  std::vector<thread_context> contexts(thread_count);

  init_thread_context(contexts, item_count, &working_package, package_size);

  // Call functions by context.
  for (size_t i = 0; i < thread_count - 1; ++i) {
    std::forward<ThreadPool>(pool).schedule([fn = EF_FORWARD(fn), &contexts, i]() mutable {
      std::invoke(EF_FORWARD(fn), contexts.data() + i);
    });
  }
  std::invoke(EF_FORWARD(fn), &contexts.back());

  std::forward<ThreadPool>(pool).wait();
}

} // namespace eflib
