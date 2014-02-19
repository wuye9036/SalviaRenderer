#pragma once

#include <eflib/include/platform/config.h>

#if defined(EFLIB_WINDOWS)
#	define EFLIB_SYM_EXPORT __declspec(dllexport)
#	define EFLIB_SYM_IMPORT __declspec(dllimport)
#endif

#if defined(EFLIB_LINUX)
#	define EFLIB_SYM_EXPORT __attribute__ ((visibility("default")))
#	define EFLIB_SYM_IMPORT
#endif
