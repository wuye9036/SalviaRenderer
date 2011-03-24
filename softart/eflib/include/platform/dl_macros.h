#include <eflib/include/platform/config.h>

#if !defined( EFLIB_DLL_EXPORTED )

#if defined( EFLIB_SHARED_LIB_ENABLED )
#	if defined( EFLIB_WINDOWS )
#		if defined(_DLL)
#			define EFLIB_DLL_EXPORTED __declspec(dllexport)
#		else
#			define EFLIB_DLL_EXPORTED __declspec(dllimport)
#		endif
#	elif __GNUC__ >= 4
#		define EFLIB_DLL_EXPORTED __attribute__((__visibility__("default")))
#	endif
#endif

#if !defined(EFLIB_DLL_EXPORTED)
#	define EFLIB_DLL_EXPORTED
#endif

#endif