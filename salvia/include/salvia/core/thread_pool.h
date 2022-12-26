#pragma once

#include <eflib/platform/config.h>
#include <eflib/thread_pool/threadpool.h>

namespace salviar {

eflib::thread_pool& global_thread_pool();

END_NS_SALVIAR()
