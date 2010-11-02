#include <eflib/include/platform/config.h>
#include "../include/cpuinfo.h"

#include <eflib/include/platform/disable_warnings.h>
#include "../include/thread_pool.h"
#include <eflib/include/platform/enable_warnings.h>

BEGIN_NS_SOFTART();

boost::threadpool::pool& global_thread_pool()
{
	static boost::threadpool::pool tp(num_available_threads() - 1);
	return tp;
}

END_NS_SOFTART();
