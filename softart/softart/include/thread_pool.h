#ifndef SOFTART_THREAD_POOL_H
#define SOFTART_THREAD_POOL_H
#ifdef SOFTART_MULTITHEADING_ENABLED

#include "eflib/include/platform.h"

#ifdef EFLIB_MSVC
#pragma warning(push)
#pragma warning(disable: 4244 4267 4512)
#endif
#include "boost/threadpool.hpp"
#ifdef EFLIB_MSVC
#pragma warning(pop)
#endif

#include "softart_fwd.h"
BEGIN_NS_SOFTART()


boost::threadpool::pool& global_thread_pool();

END_NS_SOFTART()

#endif
#endif