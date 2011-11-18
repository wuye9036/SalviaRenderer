#ifndef EFLIB_PLATFORM_CPUINFO_H
#define EFLIB_PLATFORM_CPUINFO_H

#include <eflib/include/platform/typedefs.h>

namespace eflib{
	enum instruction_sets{
		ins_none,
		
		ins_intel,
		ins_sse2,
		ins_sse3,
		ins_sse3atom,
		ins_ssse3,
		ins_sse41,
		ins_sse42,
		ins_sse4a,
		ins_avx,
		
		ins_arm,
		ins_neon,
		ins_unknown
	};

	bool support_instruction_set( instruction_sets insset );
	uint32_t num_cpu_cores();
	uint32_t num_available_threads();
}

#endif
