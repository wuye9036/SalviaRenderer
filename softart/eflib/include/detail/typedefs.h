#ifndef EFLIB_TYPEDEFS_H
#define EFLIB_TYPEDEFS_H

#include "../config.h"

#include <boost/cstdint.hpp>
	
using boost::int8_t;
using boost::int16_t;
using boost::int32_t;
using boost::int64_t;

using boost::uint8_t;
using boost::uint16_t;
using boost::uint32_t;
using boost::uint64_t;

typedef uint8_t		byte;
typedef uint16_t		word;
typedef uint32_t		dword;
typedef uint64_t		qword;

#define STATIC_ASSERT_INFO(info) static const bool info = false;

#endif

