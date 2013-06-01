#pragma once

#ifndef SALVIAR_SHADERREGS_OP_H
#define SALVIAR_SHADERREGS_OP_H

#include <salviar/include/salviar_forward.h>
#include <eflib/include/math/vector.h>

BEGIN_NS_SALVIAR();

struct viewport;
class  vs_output;

class triangle_info
{
	friend class cpp_pixel_shader;

	const eflib::vec4* pbase_vert;
	const vs_output* pddx;
	const vs_output* pddy;

	const eflib::vec4& base_vert() const;
	const vs_output& ddx() const;
	const vs_output& ddy() const;

public:
	void set(const eflib::vec4& base_vert, const vs_output& ddx, const vs_output& ddy);
};

END_NS_SALVIAR();

#endif