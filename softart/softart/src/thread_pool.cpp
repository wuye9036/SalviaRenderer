#include "eflib/include/eflib.h"

#include "../include/cpuinfo.h"
#ifdef EFLIB_MSVC
#pragma warning(push)
#pragma warning(disable : 6011)
#endif
#include "../include/thread_pool.h"
#ifdef EFLIB_MSVC
#pragma warning(pop)
#endif
BEGIN_NS_SOFTART()

boost::threadpool::pool& global_thread_pool()
{
	static boost::threadpool::pool tp(num_available_threads() - 1);
	return tp;
}

END_NS_SOFTART()
