#include <eflib/platform/config.h>
#include <eflib/platform/cpuinfo.h>

#include <eflib/concurrency/thread_pool/threadpool.h>

namespace salvia::core {

eflib::thread_pool& global_thread_pool() {
  static eflib::thread_pool tp(std::thread::hardware_concurrency() - 1);
  return tp;
}

}  // namespace salvia::core
