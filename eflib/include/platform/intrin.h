#pragma once

#include <eflib/include/platform/config.h>
#include <eflib/include/platform/typedefs.h>

#if !defined(EFLIB_CPU_X86) && !defined(EFLIB_CPU_X64)
#	error "Unsupported platform!"
#endif

#if defined(EFLIB_MSVC)
#	include <intrin.h>
#elif defined(EFLIB_MINGW) || defined(EFLIB_GCC)
#	include <cpuid.h>
#	include <x86intrin.h>
#else
#	error "Unsupported compiler!"
#endif

#if defined(EFLIB_MSVC)
uint8_t _bit_scan_reverse(uint32_t* index, uint32_t mask)
{
	return _BitScanReverse( (unsigned long*)index, mask );
}

uint8_t _bit_scan_forward(uint32_t* index, uint32_t mask)
{
	return _BitScanForward( (unsigned long*)index, mask );
}

uint8_t _bit_scan_reverse(uint32_t* index, uint64_t mask)
{
	return _BitScanReverse64( (unsigned long*)index, mask );
}

uint8_t _bit_scan_forward(uint32_t* index, uint64_t mask)
{
	return _BitScanForward64( (unsigned long*)index, mask );
}
#elif defined(EFLIB_MINGW) || defined(EFLIB_GCC)
uint8_t _bit_scan_reverse(uint32_t* index, uint32_t mask)
{
	if(mask == 0)
	{
		return 0;
	}
	*index = 31 - __builtin_clz(mask);
	return 1;
}

uint8_t _bit_scan_forward(uint32_t* index, uint32_t mask)
{
	if(mask == 0)
	{
		return 0;
	}
	*index = __builtin_ctz(mask);
	return 1;
}

uint8_t _bit_scan_reverse(uint32_t* index, uint64_t mask)
{
	if(mask == 0)
	{
		return 0;
	}
	*index = 63 - __builtin_clzll(mask);
	return 1;
}

uint8_t _bit_scan_forward(uint32_t* index, uint64_t mask)
{
	if(mask == 0)
	{
		return 0;
	}
	*index = __builtin_ctzll(mask);
	return 1;
}
#endif

inline float _xmm_extract_ps(__m128 v, int const i)
{
	EFLIB_ALIGN(16) float f[4];
	_mm_store_ps(f, v);
	return f[i];
}

inline __m128 _xmm_insert_ps(__m128 v, float s, int const i)
{
	EFLIB_ALIGN(16) float f[4];
	_mm_store_ps(f, v);
	f[i] = s;
	return _mm_load_ps(f);
}
