#ifndef SOFTART_THREAD_POOL_H
#define SOFTART_THREAD_POOL_H

#include <eflib/include/platform/config.h>

#include <eflib/include/platform/disable_warnings.h>
#include "boost/threadpool.hpp"
#include <eflib/include/platform/disable_warnings.h>

#include "softart_fwd.h"

BEGIN_NS_SOFTART();
boost::threadpool::pool& global_thread_pool();
END_NS_SOFTART();

#endif
