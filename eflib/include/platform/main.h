#pragma once

#include <eflib/include/platform/config.h>

#if defined(EFLIB_WINDOWS)
	#include <tchar.h>
	#define EFLIB_MAIN(argc, argv) int _tmain(int argc, _TCHAR* argv[])
#else
	#define EFLIB_MAIN(argc, argv) int main(int argc, char* argv[])
#endif