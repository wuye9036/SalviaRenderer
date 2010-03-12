#ifndef SOFTART_THREAD_POOL_H
#define SOFTART_THREAD_POOL_H

#include "boost/threadpool.hpp"
#include "softart_fwd.h"
BEGIN_NS_SOFTART()


boost::threadpool::pool& global_thread_pool();

END_NS_SOFTART()

#endif
