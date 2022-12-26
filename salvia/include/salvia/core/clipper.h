#pragma once

#include <salviar/include/salviar_forward.h>

#include <salviar/include/decl.h>
#include <salvia/common/constants.h>

#include <eflib/memory/pool.h>
#include <eflib/math/math.h>

#include <array>
#include <vector>

namespace salviar{

class vs_output;
struct vs_output_op;

const size_t plane_num = 2;

struct clip_context
{
	clip_context();

	typedef eflib::pool::reserved_pool<vs_output> vs_output_pool;
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
	bool		is_clipped;
};

class clipper
{
private:
	typedef bool (*cull_fn) (float);
	typedef void (clipper::*clip_impl_fn)(vs_output**, clip_results*);
	typedef eflib::pool::reserved_pool<vs_output> vs_output_pool;
	
	std::array<eflib::vec4, plane_num>	planes_;
	
	clip_context ctxt_;
	clip_impl_fn clip_impl_;

	void clip_triangle_to_poly_general (vs_output** tri_verts, clip_results*) const;
	void clip_triangle_to_poly_simple  (vs_output** tri_verts, clip_results*) const;

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

}