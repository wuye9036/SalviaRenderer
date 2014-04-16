#pragma once

#ifndef SALVIAR_SHADERREGS_OP_H
#define SALVIAR_SHADERREGS_OP_H

#include <salviar/include/salviar_forward.h>
#include <eflib/include/math/vector.h>

BEGIN_NS_SALVIAR();

struct viewport;
class  vs_output;

struct triangle_info
{
	vs_output const*			v0;
	EFLIB_ALIGN(16)	eflib::vec4	bounding_box;
	EFLIB_ALIGN(16)	eflib::vec4	edge_factors[3];
	vs_output					ddx;
	vs_output					ddy;

	triangle_info() {}
	triangle_info(triangle_info const& /*rhs*/)
	{
	}
	triangle_info& operator = (triangle_info const& /*rhs*/)
	{
		return *this;
	}
};

END_NS_SALVIAR();

#endif