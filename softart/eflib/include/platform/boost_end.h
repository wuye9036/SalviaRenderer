#if !defined(EFLIB_BOOST_BEGIN)
#	error "Please include boost_begin.h before include this file."
#else
#	undef EFLIB_BOOST_BEGIN
#endif

#if defined(EFLIB_UNDEF_STDC_LIMIT_MACROS_FOR_FIXING_WARNINGS_ON_MINGW)
#	define __STDC_LIMIT_MACROS
#endif

#include <eflib/include/platform/enable_warnings.h>
