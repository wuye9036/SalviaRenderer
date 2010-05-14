#include "eflib/include/eflib.h"
#include "../include/cpuinfo.h"

#ifdef EFLIB_WINDOWS
#	ifndef NOMINMAX
#		define NOMINMAX
#	endif
#include <windows.h>
#endif
BEGIN_NS_SOFTART()


uint32_t num_cpu_cores()
{
	static uint32_t num = 0;

#ifdef EFLIB_WINDOWS
	if (0 == num)
	{
		SYSTEM_INFO si;
		::GetSystemInfo(&si);
		num = si.dwNumberOfProcessors;
	}
#else
	if (0 == num)
	{
		// Linux doesn't easily allow us to look at the Affinity Bitmask directly,
		// but it does provide an API to test affinity maskbits of the current process
		// against each logical processor visible under OS.
		num = sysconf(_SC_NPROCESSORS_CONF);	// This will tell us how many CPUs are currently enabled.
	}
#endif

	return num;
}

uint32_t num_available_threads()
{
#ifdef SOFTART_MAX_NUM_THREADS
	return std::min(static_cast<uint32_t>(SOFTART_MAX_NUM_THREADS), num_cpu_cores());
#else
	return num_cpu_cores();
#endif
}

END_NS_SOFTART()
