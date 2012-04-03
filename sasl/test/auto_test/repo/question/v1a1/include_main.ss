#include "include_header.ss"
#include <virtual_include.ss>

#if defined(FAILED_INCLUDE)
#include <failed_include.ss>
#endif

float main_add(float a, float b)
{
	return a+b;
}