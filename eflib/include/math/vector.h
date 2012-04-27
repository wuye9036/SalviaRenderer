#ifndef EFLIB_MATH_VECTOR_H
#define EFLIB_MATH_VECTOR_H

#include <eflib/include/platform/typedefs.h>
#include <eflib/include/math/vector_generic.h>
#include <eflib/include/math/write_mask.h>
#include <eflib/include/memory/allocator.h>
#include <eflib/include/diagnostics/assert.h>
#include <cmath>

namespace eflib{

#ifndef EFLIB_NO_SIMD
	typedef ALIGN16 float float4[4];
	typedef float4 float4x4[4];
#endif

	typedef vector_<float, 2>	vec2;
	typedef vector_<float, 3>	vec3;
	typedef vector_<float, 4>	vec4;	
	typedef vector_<float, 4>	coord;

	typedef vector_<int, 2>		int2;
	typedef vector_<int, 3>		int3;
	typedef vector_<int, 4>		int4;

#ifdef EFLIB_NO_SIMD
	typedef vector_<float, 4> float4;
#endif

} //namespace
#endif
