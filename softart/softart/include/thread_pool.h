#ifndef SOFTART_THREAD_POOL_H
#define SOFTART_THREAD_POOL_H
#ifdef SOFTART_MULTITHEADING_ENABLED
#include "boost/threadpool.hpp"
#include "softart_fwd.h"
BEGIN_NS_SOFTART()


boost::threadpool::pool& global_thread_pool();

END_NS_SOFTART()

#endif
#endif