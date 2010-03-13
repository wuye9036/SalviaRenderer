#ifdef SOFTART_MULTITHEADING_ENABLED
#include "eflib/include/eflib.h"
#include "../include/cpuinfo.h"
#include "../include/thread_pool.h"
BEGIN_NS_SOFTART()

boost::threadpool::pool& global_thread_pool()
{
	static boost::threadpool::pool tp(num_cpu_cores());
	return tp;
}

END_NS_SOFTART()
#endif