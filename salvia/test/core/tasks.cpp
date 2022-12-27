#include <gtest/gtest.h>

#include <eflib/utility/extension.h>

#include <array>
#include <functional>
#include <vector>

// using namespace salvia::core;

constexpr size_t thread_count = 4;

struct pool_mock {
  template <typename F> void schedule(F &&job) { tasks_.push_back(std::forward<F>(job)); }

  void wait() const {
    for (auto const &task : tasks_) {
      task();
    }
  }
  std::vector<std::function<void()>> tasks_;
};

struct context_mock {
  size_t mock_data = 2;
};

// Test there is no ownership issue when forwarding function and executors.
template <typename Executor, typename ThreadFunc> void execute_test(Executor &&e, ThreadFunc &&f) {
  std::array<context_mock, thread_count> contexts;
  for (size_t i = 0; i < thread_count - 1; ++i) {
    EF_FORWARD(e).schedule([f = EF_FORWARD(f), &contexts, i]() mutable {
      std::invoke(EF_FORWARD(f), contexts.data() + i);
    });
  }
  std::invoke(EF_FORWARD(f), &contexts.back());
  EF_FORWARD(e).wait();
}

TEST(salvia_core, task_forward) {
  size_t execution_counter{0};
  pool_mock executor;
  execute_test(executor, [&execution_counter](context_mock const *ctx) {
    execution_counter += ctx->mock_data;
  });

  EXPECT_EQ(execution_counter, 8);
}