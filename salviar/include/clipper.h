#pragma once

#include <salviar/include/salviar_forward.h>

#include <salviar/include/decl.h>
#include <salviar/include/enums.h>

#include <eflib/include/memory/pool.h>
#include <eflib/include/math/math.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/array.hpp>
#include <eflib/include/platform/boost_end.h>

#include <vector>

BEGIN_NS_SALVIAR();

const size_t plane_num = 2;

struct clip_context
{
	clip_context();

	typedef eflib::pool::preserved_pool<vs_output> vs_output_pool;
	typedef bool (*cull_fn) (float);

	vs_output_pool*		vert_pool;
	prim_type			prim;
	vs_output_op const*	vso_ops;
	cull_fn				cull;
};

struct clip_results
{
	vs_output**	clipped_verts;
	uint32_t	num_clipped_verts;
	bool		is_front;
};

class clipper
{
private:
	typedef bool (*cull_fn) (float);
	typedef void (clipper::*clip_impl_fn)(vs_output**, clip_results*);
	typedef eflib::pool::preserved_pool<vs_output> vs_output_pool;
	
	boost::array<eflib::vec4, plane_num>	planes_;
	
	clip_context ctxt_;
	clip_impl_fn clip_impl_;

	void clip_triangle_to_poly	(vs_output** tri_verts, clip_results*) const;

	void clip_wireframe_triangle(vs_output** tri_verts, clip_results* rslt);
	void clip_solid_triangle	(vs_output** tri_verts, clip_results* rslt);

public:
	clipper();

	void set_context(clip_context const* ctxt);

	inline void clip(vs_output** tri_verts, clip_results* rslt)
	{
		(this->*clip_impl_)(tri_verts, rslt);
	}
};

END_NS_SALVIAR();