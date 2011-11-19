#ifndef EFLIB_PLATFORM_CPUINFO_H
#define EFLIB_PLATFORM_CPUINFO_H

#include <eflib/include/platform/typedefs.h>

namespace eflib{
	enum cpu_features{
		cpu_none,
		
		cpu_intel,
		cpu_sse2,
		cpu_sse3,
		cpu_sse3atom,
		cpu_ssse3,
		cpu_sse41,
		cpu_sse42,
		cpu_sse4a,
		cpu_avx,
		
		cpu_arm,
		cpu_neon,
		cpu_unknown
	};

	bool support_feature( cpu_features );
	uint32_t num_cpu_cores();
	uint32_t num_available_threads();
}

#endif
