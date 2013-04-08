#ifndef SALVIAR_CLIPPER_H
#define SALVIAR_CLIPPER_H

#include <salviar/include/decl.h>
#include <salviar/include/salviar_forward.h>

#include <eflib/include/memory/pool.h>

#include <eflib/include/math/math.h>
#include <boost/array.hpp>
#include <vector>

BEGIN_NS_SALVIAR()

const size_t plane_num = 2;

struct clip_context
{
	typedef eflib::pool::preserved_pool<vs_output> vs_output_pool;

	/* Output */
	vs_output**			clipped_verts;
	uint32_t*			num_clipped_verts;
	bool*				is_front;

	/* Input */
	bool				(*cull_fn)(float);
	vs_output_pool*		vso_pool;
	vs_output*			prim_verts[3];
	vs_output_op const*	vs_output_ops;
};

class clipper
{
	boost::array<eflib::vec4, plane_num>				planes_;

public:
	clipper();

	void clip_triangle(clip_context const*) const;
};

END_NS_SALVIAR()

#endif