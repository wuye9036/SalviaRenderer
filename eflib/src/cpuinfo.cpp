#include <eflib/platform/config.h>
#include <eflib/platform/cpuinfo.h>
#include <eflib/platform/stdint.h>

#include <algorithm>

#if defined(EFLIB_CPU_X64)
#include <intrin.h>
#endif

namespace eflib {
#if defined(EFLIB_CPU_X64)

class x86_cpuinfo {
public:
  x86_cpuinfo() {
    memset(feats, 0, sizeof(feats));

    int cpu_infos[4];
    int cpu_infos_ex[4];
#if defined(EFLIB_MSVC) || defined(EFLIB_MINGW64)
    __cpuid(cpu_infos, 1);
    __cpuid(cpu_infos_ex, 0x80000001);
#elif defined(EFLIB_MINGW32) || defined(EFLIB_GCC) || defined(EFLIB_CLANG)
    __cpuid(1, cpu_infos[0], cpu_infos[1], cpu_infos[2], cpu_infos[3]);
    __cpuid(0x80000001, cpu_infos_ex[0], cpu_infos_ex[1], cpu_infos_ex[2], cpu_infos_ex[3]);
#endif
    feats[cpu_sse2] = (cpu_infos[3] & (1 << 26)) || false;
    feats[cpu_sse3] = (cpu_infos[2] & 0x1) || false;
    feats[cpu_ssse3] = (cpu_infos[2] & 0x200) || false;
    feats[cpu_sse41] = (cpu_infos[2] & 0x80000) || false;
    feats[cpu_sse42] = (cpu_infos[2] & 0x100000) || false;
    feats[cpu_sse4a] = (cpu_infos_ex[2] & 0x40) || false;
    feats[cpu_avx] = ((cpu_infos[2] & (1 << 27)) && (cpu_infos[2] & (1 << 28))) || false;

    // others are unchecked.
  };

private:
  bool feats[cpu_unknown];

public:
  bool support(cpu_features feat) const { return feats[feat]; }
};

static x86_cpuinfo cpuinfo;

#endif

bool support_feature(cpu_features feat) { 
#if defined(EFLIB_CPU_X64)
  return cpuinfo.support(feat);
#endif
  // A fake return;
  return true;
}

} // namespace eflib
