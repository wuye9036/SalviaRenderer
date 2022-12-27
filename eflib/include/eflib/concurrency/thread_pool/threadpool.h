#pragma once

#include "./detail/pool.hpp"

#include <atomic>
#include <functional>
#include <thread>
#include <vector>

#define USE_MY_THREAD_POOL 1

namespace eflib {
#if USE_MY_THREAD_POOL == 1
class thread_pool {
public:
  explicit thread_pool(uint32_t thread_count)
      : worker_count_{thread_count}, terminate_{false}, pending_count_{0} {
    initialize();
  }

  template <typename F> void schedule(F &&job) {
    ++pending_count_;
    {
      std::unique_lock<std::mutex> lock(queue_mutex_);
      tasks_.push_back(std::forward<F>(job));
    }
    queue_cv_.notify_one();
  }

  void wait() {
    while (pending_count_ > 0) {
      std::this_thread::yield();
    }
  }

  void terminate() {
    {
      std::unique_lock<std::mutex> lock(queue_mutex_);
      terminate_ = true;
    }
    queue_cv_.notify_all();

    for (auto &worker : workers_) {
      worker.join();
    }

    workers_.clear();
    tasks_.clear();
    pending_count_ = 0;
  }

  ~thread_pool() { terminate(); }

private:
  void initialize() {
    for (uint32_t i_worker = 0; i_worker < worker_count_; ++i_worker) {
      workers_.emplace_back([this]() { thread_func(); });
    }
  }

  void thread_func() {
    std::function<void()> job;

    for (;;) {
      {
        std::unique_lock<std::mutex> lock(queue_mutex_);

        queue_cv_.wait(lock, [this] { return !tasks_.empty() || terminate_; });

        if (terminate_) {
          return;
        }

        if (!tasks_.empty()) {
          std::swap(job, tasks_.back());
          tasks_.pop_back();
        }
      }

      if (job) {
        job();
        --pending_count_;
      }
    }
  }

  bool terminate_;
  std::mutex queue_mutex_;
  std::condition_variable queue_cv_;

  uint32_t worker_count_;
  std::atomic<intptr_t> pending_count_;
  std::vector<std::thread> workers_;
  std::vector<std::function<void()>> tasks_;
};
#else
using thread_pool = threadpool::pool;
#endif
} // namespace eflib
