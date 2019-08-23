#pragma once

#include <eflib/include/platform/config.h>
#include <eflib/include/thread_pool/threadpool.h>
#include <salviar/include/salviar_forward.h>

BEGIN_NS_SALVIAR();

eflib::thread_pool::pool& global_thread_pool();

END_NS_SALVIAR();
