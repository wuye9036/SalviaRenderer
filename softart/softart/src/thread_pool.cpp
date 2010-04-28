#include "eflib/include/eflib.h"

#ifdef SOFTART_MULTITHEADING_ENABLED
#include "../include/cpuinfo.h"
#ifdef EFLIB_MSVC
#pragma warning(push)
#pragma warning(disable: 4512 4244)
#endif
#include "../include/thread_pool.h"
#ifdef EFLIB_MSVC
#pragma warning(pop)
#endif
BEGIN_NS_SOFTART()

boost::threadpool::pool& global_thread_pool()
{
	static boost::threadpool::pool tp(num_cpu_cores());
	return tp;
}

END_NS_SOFTART()
#endif