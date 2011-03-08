#ifndef EFLIB_PLATFORM_CPUINFO_H
#define EFLIB_PLATFORM_CPUINFO_H

#include <eflib/include/platform/typedefs.h>

namespace eflib{
	uint32_t num_cpu_cores();
	uint32_t num_available_threads();
}

#endif
