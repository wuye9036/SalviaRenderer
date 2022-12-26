#include <eflib/platform/config.h>
#include <eflib/platform/cpuinfo.h>

#include <eflib/thread_pool/threadpool.h>

#include <salviar/include/salviar_forward.h>

using eflib::num_available_threads;

namespace salviar {

eflib::thread_pool& global_thread_pool()
{
	static eflib::thread_pool tp(std::thread::hardware_concurrency() - 1);
	return tp;
}

END_NS_SALVIAR()
