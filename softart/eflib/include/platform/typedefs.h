#ifndef EFLIB_PLATFORM_TYPEDEFS_H
#define EFLIB_PLATFORM_TYPEDEFS_H

#include <eflib/include/platform/config.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/cstdint.hpp>
#include <eflib/include/platform/boost_end.h>

using boost::int8_t;
using boost::int16_t;
using boost::int32_t;
using boost::int64_t;

using boost::uint8_t;
using boost::uint16_t;
using boost::uint32_t;
using boost::uint64_t;

typedef boost::uint8_t byte;

#define STATIC_ASSERT_INFO(info) static const bool info = false;

#endif

