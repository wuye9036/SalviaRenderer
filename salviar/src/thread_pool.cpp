#include <eflib/include/platform/config.h>
#include <eflib/include/platform/cpuinfo.h>

#include <eflib/include/thread_pool/threadpool.h>

#include <salviar/include/salviar_forward.h>

using eflib::num_available_threads;

BEGIN_NS_SALVIAR()

eflib::threadpool::pool& global_thread_pool()
{
	static eflib::threadpool::pool tp(num_available_threads() - 1);
	return tp;
}

END_NS_SALVIAR()
