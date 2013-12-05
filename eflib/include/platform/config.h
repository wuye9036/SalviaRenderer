#ifndef EFLIB_PLATFORM_CONFIG_H
#define EFLIB_PLATFORM_CONFIG_H

#if defined(DEBUG) || defined(_DEBUG)
#	define EFLIB_DEBUG
#endif

#if defined(_UNICODE)
#define EFLIB_UNICODE
#endif

#ifdef _MSC_VER
#	define EFLIB_MSVC
#	define EFILB_COMPILE_VER _MSC_VER
#	define _SECURE_SCL 0
#	pragma warning(disable: 4251 4275 4819)
#	ifndef _CRT_SECURE_NO_DEPRECATE
#		define _CRT_SECURE_NO_DEPRECATE
#		define _CRT_SECURE_NO_WARNINGS
#	endif
#	ifndef _SCL_SECURE_NO_DEPRECATE
#		define _SCL_SECURE_NO_DEPRECATE
#		define _SCL_SECURE_NO_WARNINGS
#	endif
#elif defined(__MINGW32__)
#	include <_mingw.h>
#	define EFLIB_MINGW
#	if defined(__MINGW64_VERSION_MAJOR)
#		define EFLIB_MINGW64
		// Fix the C++ linkage problem in GCC 4.8.2
#   	include <intrin.h>
#	else
#		define EFLIB_MINGW32
#	endif
#elif defined( __GNUC__ )
#   define EFLIB_GCC
#endif

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#	define EFLIB_WINDOWS
#endif

#ifdef EFLIB_WINDOWS
#	ifndef BOOST_ALL_DYN_LINK
#		define BOOST_ALL_DYN_LINK
#	endif
#endif

#ifdef EFLIB_DEBUG
#	ifndef SOFTART_MAX_NUM_THREADS
#		define SOFTART_MAX_NUM_THREADS 4
#	endif
#endif

#if defined(EFLIB_MSVC)
	#if defined(_M_X64)
		#define EFLIB_CPU_X64
		#define EFLIB_COMPILER_TARGET x64
	#elif defined(_M_IX86)
		#define EFLIB_CPU_X86
		#define EFLIB_COMPILER_TARGET x86
	#else
		#error Unknown CPU type.
	#endif
#elif defined(EFLIB_GCC) || defined(EFLIB_MINGW)
	#if defined(__x86_64__)
		#define EFLIB_CPU_X64
		#define EFLIB_COMPILER_TARGET x64
	#elif defined(__i386__)
		#define EFLIB_CPU_X86
		#define EFLIB_COMPILER_TARGET x86
	#else
		#error Unknown CPU type.
	#endif
#endif

#ifdef EFLIB_MSVC
#	define EFLIB_ALIGN(x)	__declspec(align( x ))
#else
#	define EFLIB_ALIGN(x)   __attribute__ ((aligned ( x )))
#endif
#endif
