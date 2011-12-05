#include <salviar/include/shader_abi.h>

int salviar::SIMD_WIDTH_IN_BYTES()
{
	// TODO if avx is supported, it will return 32.
	return 16;
}
