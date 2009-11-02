#ifndef EFLIB_CONFIG_H
#define EFLIB_CONFIG_H

#include "user_config.h"
#include <boost/config.hpp>

#ifdef BOOST_MSVC
#	define EFLIB_MSVC
#	define EFILB_COMPILE_VER _MSC_VER
#	ifdef _UNICODE
#		define EFLIB_UNICODE
#	endif
#endif

#ifdef BOOST_WINDOWS
#	define EFLIB_WINDOWS
#endif

#endif