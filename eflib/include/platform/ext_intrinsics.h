#ifndef EFLIB_PLATFORM_EXT_INTRINSICS_H
#define EFLIB_PLATFORM_EXT_INTRINSICS_H

#include <eflib/include/platform/config.h>

#include <emmintrin.h>
#include <xmmintrin.h>

inline __m128 eflib_mm_castsi128_ps( __m128i v ){
#if defined(EFLIB_MSVC) && EFLIB_COMPILE_VER >= 1500
	// MSVC 2008 or later
	return _mm_castsi128_ps(v);
#else
	// MSVC 2005 or former
	return *( reinterpret_cast<__m128*>(&v) );
#endif
}

inline __m128i eflib_mm_castps_si128( __m128 v ){
#if defined(EFLIB_MSVC) && EFLIB_COMPILE_VER >= 1500
	// MSVC 2008 or later
	return _mm_castps_si128(v);
#else
	// MSVC 2005 or former
	return *( reinterpret_cast<__m128i*>(&v) );
#endif
}

#endif