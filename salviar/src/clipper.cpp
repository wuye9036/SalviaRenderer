#include <salviar/include/clipper.h>

#include <salviar/include/shader_regs.h>
#include <salviar/include/shader_regs_op.h>
#include <salviar/include/shader.h>

#include <eflib/include/memory/pool.h>

#include <algorithm>

BEGIN_NS_SALVIAR();

using namespace eflib;
using namespace std;

clip_context::clip_context()
	: vert_pool(NULL), prim(pt_none), vso_ops(NULL), cull(NULL)
{
}

clipper::clipper()
{
	// Near plane is 0.
	planes_[0] = vec4(0.0f, 0.0f, 1.0f, 0.0f);

	// Far plane
	planes_[1] = vec4(0.0f, 0.0f, -1.0f, 1.0f);
}

void clipper::set_context(clip_context const* ctxt)
{
	ctxt_ = *ctxt;

	// Select clipping function
	switch(ctxt->prim)
	{
	case pt_solid_tri:
		clip_impl_ = &clipper::clip_solid_triangle;
		break;
	default:
		EFLIB_ASSERT_UNIMPLEMENTED();
	}
}

void clipper::clip_wireframe_triangle(vs_output** /*tri_verts*/, clip_results* /*results*/)
{
}

void clipper::clip_solid_triangle(vs_output** tri_verts, clip_results* results)
{
	// Clip triangles to vertex of result polygon
	vs_output*	tri_clipped_verts[5];
	clip_results tri_clip_results;
	tri_clip_results.clipped_verts = tri_clipped_verts;
	clip_triangle_to_poly(tri_verts, &tri_clip_results);

	// Re-topo/subdivde polygon to triangles.
	assert(tri_clip_results.num_clipped_verts <= 5);
	if (tri_clip_results.num_clipped_verts < 3 )
	{
		results->num_clipped_verts = 0;
		return;
	}

	results->num_clipped_verts = (tri_clip_results.num_clipped_verts - 2) * 3;
	results->is_front = tri_clip_results.is_front;

	for (uint32_t i = 0; i < tri_clip_results.num_clipped_verts; ++i)
	{
		tri_clip_results.clipped_verts[i]->front_face(tri_clip_results.is_front);
	}

	vs_output** clipped_cursor = results->clipped_verts;
	for(size_t i_tri = 1; i_tri < results->num_clipped_verts-1; ++i_tri)
	{
		*(clipped_cursor+0) = tri_clipped_verts[0];
		if (results->is_front)
		{
			*(clipped_cursor+1) = tri_clipped_verts[i_tri];
			*(clipped_cursor+2) = tri_clipped_verts[i_tri+1];
		}
		else
		{
			*(clipped_cursor+1) = tri_clipped_verts[i_tri+1];
			*(clipped_cursor+2) = tri_clipped_verts[i_tri];
		}
		clipped_cursor += 3;
	}
}

inline bool compute_front(vec4 const& pos0, vec4 const& pos1, vec4 const& pos2)
{
	vec2 pv_2d[3] =
	{
		pos0.xy() * ( 1.0f / pos0.w() ),
		pos1.xy() * ( 1.0f / pos1.w() ),
		pos2.xy() * ( 1.0f / pos2.w() ),
	};

	float const area = cross_prod2(pv_2d[2] - pv_2d[0], pv_2d[1] - pv_2d[0]);
	return area > 0.0f;
}

void clipper::clip_triangle_to_poly(vs_output** tri_verts, clip_results* results) const
{
	vs_output*	clipped_verts[2][5];
	uint32_t	num_clipped_verts[2];

	// Quick test
	bool in_frustum = true;
	for(size_t i_plane = 0; i_plane < planes_.size(); ++i_plane)
	{
		for(size_t i_vert = 0; i_vert < 3; ++i_vert)
		{
			float d = dot_prod4( planes_[i_plane], tri_verts[i_vert]->position() );
			if(d < 0)
			{
				in_frustum = false;
				break;
			}
		}
		if (!in_frustum) break;
	}

	if(in_frustum)
	{
		results->is_front = compute_front(
			tri_verts[0]->position(),
			tri_verts[1]->position(),
			tri_verts[2]->position()
			);
		// If triangle is culled, return 0.
		if( ctxt_.cull(results->is_front ? 1.0 : -1.0) )
		{
			results->num_clipped_verts = 0;
			return;
		}

		results->num_clipped_verts = 3;
		for(size_t i = 0; i < 3; ++i)
		{
			results->clipped_verts[i] = tri_verts[i];
		}
		
		return;
	}

	// clip by all faces as Ping-Pong idiom
	clipped_verts[0][0] = tri_verts[0];
	clipped_verts[0][1] = tri_verts[1];
	clipped_verts[0][2] = tri_verts[2];
	num_clipped_verts[0] = 3;

	float d[2];
	size_t src_stage = 0;
	size_t dest_stage = 1;

	for(size_t i_plane = 0; i_plane < planes_.size(); ++i_plane)
	{
		num_clipped_verts[dest_stage] = 0;

		if (num_clipped_verts[src_stage] != 0)
		{
			d[0] = dot_prod4( planes_[i_plane], clipped_verts[src_stage][0]->position() );
		}

		for(size_t i = 0, j = 1; i < num_clipped_verts[src_stage]; ++i, ++j)
		{
			//wrap
			j %= num_clipped_verts[src_stage];

			d[1] = dot_prod4( planes_[i_plane], clipped_verts[src_stage][j]->position() );

			if(d[0] >= 0.0f)
			{
				clipped_verts[dest_stage][num_clipped_verts[dest_stage]] = clipped_verts[src_stage][i];
				++ num_clipped_verts[dest_stage];

				if(d[1] < 0.0f)
				{
					vs_output* pclipped = ctxt_.vert_pool->alloc();

					//LERP
					ctxt_.vso_ops->lerp(*pclipped, *clipped_verts[src_stage][i], *clipped_verts[src_stage][j], d[0] / (d[0] - d[1]));

					clipped_verts[dest_stage][num_clipped_verts[dest_stage]] = pclipped;
					++ num_clipped_verts[dest_stage];
				}
			}
			else
			{
				if(d[1] >= 0.0f)
				{
					vs_output* pclipped = ctxt_.vert_pool->alloc();

					//LERP
					ctxt_.vso_ops->lerp(*pclipped, *clipped_verts[src_stage][j], *clipped_verts[src_stage][i], d[1] / (d[1] - d[0]));

					clipped_verts[dest_stage][num_clipped_verts[dest_stage]] = pclipped;
					++ num_clipped_verts[dest_stage];
				}
			}

			d[0] = d[1];
		}

		if ( i_plane == 0 )
		{
			// Do some special after clipped by near plane
			if ( num_clipped_verts[dest_stage] >= 3 )
			{
				vec2 pv_2d[3];
				for (size_t i = 0; i < 3; ++i)
				{	
					vs_output& v = *clipped_verts[dest_stage][i];
					float const inv_abs_w = 1 / abs( v.position().w() );
					float const x = v.position().x() * inv_abs_w;
					float const y = v.position().y() * inv_abs_w;
					pv_2d[i] = vec2(x, y);
				}

				float const area = cross_prod2(pv_2d[2] - pv_2d[0], pv_2d[1] - pv_2d[0]);
				results->is_front = area > 0.0f;

				// If triangle is culled, return 0.
				if( ctxt_.cull(area) )
				{
					results->num_clipped_verts = 0;
					return;
				}
			}
		}

		//swap src and dest pool
		++src_stage;	src_stage &= 1;
		++dest_stage;	dest_stage &= 1;
	}

	uint32_t num_final_clipped_verts = num_clipped_verts[src_stage];
	assert(num_final_clipped_verts <= 5);

	results->num_clipped_verts = num_final_clipped_verts;
	for(size_t i = 0; i < num_final_clipped_verts; ++i)
	{
		results->clipped_verts[i] = clipped_verts[src_stage][i];
	}
}

END_NS_SALVIAR();
