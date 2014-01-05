#pragma once

#include <eflib/include/platform/config.h>

#if defined(EFLIB_WINDOWS)
#	define EFLIB_SYM_EXPORT __declspec(export)
#	define EFLIB_SYM_IMPORT __declspec(import)
#endif

#if defined(EFLIB_LINUX)
#	define EFLIB_SYM_EXPORT __attribute__ ((visibility("default")))
#	define EFLIB_SYM_IMPORT
#endif
