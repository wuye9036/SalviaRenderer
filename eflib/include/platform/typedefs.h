#ifndef EFLIB_PLATFORM_TYPEDEFS_H
#define EFLIB_PLATFORM_TYPEDEFS_H

#include <eflib/include/platform/config.h>

#include <cstdint>

using std::int8_t;
using std::int16_t;
using std::int32_t;
using std::int64_t;

using std::uint8_t;
using std::uint16_t;
using std::uint32_t;
using std::uint64_t;

#define STATIC_ASSERT_INFO(info) static const bool info = false;

#endif

