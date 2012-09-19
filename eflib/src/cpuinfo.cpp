#include <eflib/include/platform/config.h>
#include <eflib/include/platform/cpuinfo.h>

#include <eflib/include/platform/typedefs.h>

#if defined( EFLIB_WINDOWS ) && defined( EFLIB_MSVC )
#	include <intrin.h>
#else
#	error	"Can not support other compiler than MSVC yet."
#endif

#ifdef EFLIB_WINDOWS
#	ifndef NOMINMAX
#		define NOMINMAX
#	endif
#include <windows.h>
#endif

#include <algorithm>

namespace eflib{
#if defined( EFLIB_CPU_X64 ) || defined( EFLIB_CPU_X86 )

	class x86_cpuinfo{
	public:
		x86_cpuinfo(){
			memset( feats, 0, sizeof(feats) );

			int cpu_infos[4];
			int cpu_infos_ex[4];

#if defined(EFLIB_MSVC)
			__cpuid(cpu_infos, 1);
			__cpuid(cpu_infos_ex, 0x80000001);
#else
#	error "Unsupported compiler."
#endif
			feats[cpu_sse2]		= ( cpu_infos[3] & (1 << 26) ) || false;
			feats[cpu_sse3]		= ( cpu_infos[2] & 0x1 ) || false;
			feats[cpu_ssse3]	= ( cpu_infos[2] & 0x200 ) || false;
			feats[cpu_sse41]	= ( cpu_infos[2] & 0x80000) || false;
			feats[cpu_sse42]	= ( cpu_infos[2] & 0x100000) || false;
			feats[cpu_sse4a]	= ( cpu_infos_ex[2] & 0x40) || false;
			feats[cpu_avx]		= (( cpu_infos[2] & (1 << 27) ) && ( cpu_infos[2] & (1 << 28) )) || false;

			// others are unchecked.
		};
	private:
		bool feats[cpu_unknown];
	public:
		bool support( cpu_features feat ) const{
			return feats[feat];
		}
	};

	static x86_cpuinfo cpuinfo;

#endif

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
#if defined( EFLIB_DEBUG )
		return 1;
#else
		return num_cpu_cores();
#endif
	}

	bool support_feature( cpu_features feat )
	{
		return cpuinfo.support(feat);
	}

}
