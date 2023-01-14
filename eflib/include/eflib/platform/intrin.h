#pragma once

#include <eflib/platform/config.h>
#include <eflib/platform/stdint.h>

#if !defined(SIMDE_ENABLE_NATIVE_ALIASES)
#define SIMDE_ENABLE_NATIVE_ALIASES 1
#endif

#include <simde/x86/sse.h>
#include <simde/x86/sse2.h>

#if defined(EFLIB_MSVC)
inline uint8_t _xmm_bsr(uint32_t *index, uint32_t mask) {
  return _BitScanReverse((unsigned long *)index, mask);
}

inline uint8_t _xmm_bsf(uint32_t *index, uint32_t mask) {
  return _BitScanForward((unsigned long *)index, mask);
}

inline uint8_t _xmm_bsr(uint32_t *index, uint64_t mask) {
#if defined(EFLIB_CPU_X64)
  return _BitScanReverse64((unsigned long *)index, mask);
#else
  if (mask == 0)
    return 0;
  uint64_t indicator = 1;
  *index = 0;
  while (indicator < mask) {
    ++(*index);
    indicator <<= 1;
  }
  return 1;
#endif
}

inline uint8_t _xmm_bsf(uint32_t *index, uint64_t mask) {
#if defined(EFLIB_CPU_X64)
  return _BitScanForward64((unsigned long *)index, mask);
#else
  if (mask == 0)
    return 0;
  uint64_t indicator = 1;
  *index = 0;
  while ((indicator & mask) == 0) {
    ++(*index);
    indicator <<= 1;
  }
  return 1;
#endif
}
#elif defined(EFLIB_GCC) || defined(EFLIB_CLANG)
inline uint8_t _xmm_bsr(uint32_t *index, uint32_t mask) {
  if (mask == 0) {
    return 0;
  }
  *index = 31 - __builtin_clz(mask);
  return 1;
}

inline uint8_t _xmm_bsf(uint32_t *index, uint32_t mask) {
  if (mask == 0) {
    return 0;
  }
  *index = __builtin_ctz(mask);
  return 1;
}

inline uint8_t _xmm_bsr(uint32_t *index, uint64_t mask) {
  if (mask == 0) {
    return 0;
  }
  *index = 63 - __builtin_clzll(mask);
  return 1;
}

inline uint8_t _xmm_bsf(uint32_t *index, uint64_t mask) {
  if (mask == 0) {
    return 0;
  }
  *index = __builtin_ctzll(mask);
  return 1;
}
#endif

inline float _xmm_extract_ps(__m128 v, int const i) {
  EFLIB_ALIGN(16) float f[4];
  _mm_store_ps(f, v);
  return f[i];
}

inline __m128 _xmm_insert_ps(__m128 v, float s, int const i) {
  EFLIB_ALIGN(16) float f[4];
  _mm_store_ps(f, v);
  f[i] = s;
  return _mm_load_ps(f);
}