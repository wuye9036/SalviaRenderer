#ifndef EFLIB_MATH_MATRIX_H
#define EFLIB_MATH_MATRIX_H

#include <eflib/include/platform/config.h>
#include <eflib/include/math/vector.h>
#include <eflib/include/math/matrix_generic.h>

namespace eflib
{
	typedef matrix_<float, 3, 3> mat33;
	typedef matrix_<float, 4, 4> mat44;
}
#endif
