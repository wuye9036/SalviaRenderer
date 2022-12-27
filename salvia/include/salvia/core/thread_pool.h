#pragma once

#include <eflib/platform/config.h>
#include <eflib/concurrency/thread_pool/threadpool.h>

namespace salvia::core {

eflib::thread_pool &global_thread_pool();

}
