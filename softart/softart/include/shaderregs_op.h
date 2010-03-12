#ifndef SOFTART_SHADERREGS_OP_H
#define SOFTART_SHADERREGS_OP_H

#include "shaderregs.h"

#include <boost/static_assert.hpp>
#include "softart_fwd.h"
BEGIN_NS_SOFTART()


struct viewport;

class triangle_info
{
	friend class pixel_shader;

	const efl::vec4* pbase_vert;
	const vs_output* pddx;
	const vs_output* pddy;

	const efl::vec4& base_vert() const;
	const vs_output& ddx() const;
	const vs_output& ddy() const;

public:
	void set(const efl::vec4& base_vert, const vs_output& ddx, const vs_output& ddy);
};

END_NS_SOFTART()

#endif