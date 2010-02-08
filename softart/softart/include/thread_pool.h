#ifndef SOFTART_THREAD_POOL_H
#define SOFTART_THREAD_POOL_H

#include "boost/threadpool.hpp"

boost::threadpool::pool& global_thread_pool();

#endif
