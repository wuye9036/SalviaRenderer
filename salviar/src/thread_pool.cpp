#include <eflib/include/platform/config.h>
#include <eflib/include/platform/cpuinfo.h>

#include <eflib/include/thread_pool/threadpool.h>

#include <salviar/include/salviar_forward.h>

using eflib::num_available_threads;

BEGIN_NS_SALVIAR()

eflib::thread_pool& global_thread_pool()
{
	static eflib::thread_pool tp(std::thread::hardware_concurrency() - 1);
	return tp;
}

END_NS_SALVIAR()
