#ifndef EFLIB_PLATFORM_TYPEDEFS_H
#define EFLIB_PLATFORM_TYPEDEFS_H

#include <eflib/include/platform/config.h>

#if !defined(HAVE_STDINT_H) || HAVE_STDINT_H == 0

	#if !defined(SUPPORT_DATATYPES_H) // Patch for LLVM
		#include <boost/cstdint.hpp>

		using boost::int8_t;
		using boost::int16_t;
		using boost::int32_t;
		using boost::int64_t;

		using boost::uint8_t;
		using boost::uint16_t;
		using boost::uint32_t;
		using boost::uint64_t;
	#endif
#endif

typedef uint8_t		byte;
typedef uint16_t	word;
typedef uint32_t	dword;
typedef uint64_t	qword;

#define STATIC_ASSERT_INFO(info) static const bool info = false;

#endif

