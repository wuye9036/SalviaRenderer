#include <eflib/include/platform/config.h>
#include <eflib/include/platform/cpuinfo.h>

#include <eflib/include/platform/disable_warnings.h>
#include "../include/thread_pool.h"
#include <eflib/include/platform/enable_warnings.h>

using eflib::num_available_threads;

BEGIN_NS_SOFTART();

boost::threadpool::pool& global_thread_pool()
{
	static boost::threadpool::pool tp(num_available_threads() - 1);
	return tp;
}

END_NS_SOFTART();
