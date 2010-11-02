//特意没有防卫头

//防止一些平台头文件的宏与std的函数冲突

//需要引用头文件时，先定义宏（如EFLIB_INCLUDE_WINDOWS_H），然后在包含includer.h即可

#include <eflib/include/platform/config.h>

#ifdef EFLIB_WINDOWS

#ifdef EFLIB_INCLUDE_WINDOWS_H
	#ifndef NOMINMAX
		#define NOMINMAX
	#endif
	#include <windows.h>
	#ifndef EFLIB_USE_STD_MINMAX
		#include <algorithm>
		using ::std::min;
		using ::std::max;
		#define EFLIB_USE_STD_MINMAX
	#endif
#endif

#ifdef EFLIB_INCLUDE_GDIPLUS_H
	#ifndef EFLIB_USE_STD_MINMAX
		#include <algorithm>
		using ::std::min;
		using ::std::max;
		#define EFLIB_USE_STD_MINMAX
	#endif
	#include <gdiplus.h>
#endif

#ifdef EFLIB_INCLUDE_D3D9_H
	#ifndef EFLIB_USE_STD_MINMAX
		#include <algorithm>
		using ::std::min;
		using ::std::max;
		#define EFLIB_USE_STD_MINMAX
	#endif
	#include <d3d9.h>
#endif

#ifdef EFLIB_INCLUDE_D3DX9_H
	#ifndef EFLIB_USE_STD_MINMAX
		#include <algorithm>
		using ::std::min;
		using ::std::max;
		#define EFLIB_USE_STD_MINMAX
	#endif
	#include <d3dx9.h>
#endif

#endif //config windows
