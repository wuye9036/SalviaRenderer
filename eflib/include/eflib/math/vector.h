#pragma once

#include <eflib/diagnostics/assert.h>
#include <eflib/math/vector_generic.h>
#include <eflib/math/write_mask.h>
#include <eflib/memory/allocator.h>

#include <cmath>

namespace eflib {

#ifndef EFLIB_NO_SIMD
typedef EFLIB_ALIGN(16) float float4[4];
typedef float4 float4x4[4];
#endif

using vec2 = vector_<float, 2>;
using vec3 = vector_<float, 3>;
using vec4 = vector_<float, 4>;
using coord = vector_<float, 4>;

using int2 = vector_<int, 2>;
using int3 = vector_<int, 3>;
using int4 = vector_<int, 4>;

using uint4 = vector_<uint32_t, 4>;

#ifdef EFLIB_NO_SIMD
typedef vector_<float, 4> float4;
#endif

}  // namespace eflib
