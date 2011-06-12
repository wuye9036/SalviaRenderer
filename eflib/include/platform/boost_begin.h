#include <eflib/include/platform/config.h>

#if defined(EFLIB_BOOST_BEGIN)
#	error "Include begin boost twice."
#else
#	define EFLIB_BOOST_BEGIN
#endif

// From boost/config/platform/win32.h
#if ( defined(__MINGW32__) && ((__MINGW32_MAJOR_VERSION > 2) || ((__MINGW32_MAJOR_VERSION == 2) && (__MINGW32_MINOR_VERSION >= 0))) )
#	if defined(__STDC_LIMIT_MACROS)
#		define EFLIB_UNDEF_STDC_LIMIT_MACROS_FOR_FIXING_WARNINGS_ON_MINGW
#		undef __STDC_LIMIT_MACROS
#	endif
#endif

#include <eflib/include/platform/disable_warnings.h>
