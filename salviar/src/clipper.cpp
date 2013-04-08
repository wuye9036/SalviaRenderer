#include <salviar/include/clipper.h>

#include <salviar/include/shaderregs.h>
#include <salviar/include/shaderregs_op.h>
#include <salviar/include/shader.h>

#include <eflib/include/memory/pool.h>

#include <algorithm>

BEGIN_NS_SALVIAR();

using namespace eflib;
using namespace std;

clipper::clipper(){
	// Near plane is 0.
	planes_[0] = vec4(0.0f, 0.0f, 1.0f, 0.0f);

	// Far plane
	planes_[1] = vec4(0.0f, 0.0f, -1.0f, 1.0f);
}

void clipper::clip_triangle(clip_context const* ctxt) const
{
	vs_output*	clipped_verts[2][5];
	uint32_t	num_clipped_verts[2];
	
	// clip by all faces as Ping-Pong idiom
	clipped_verts[0][0] = ctxt->prim_verts[0];
	clipped_verts[0][1] = ctxt->prim_verts[1];
	clipped_verts[0][2] = ctxt->prim_verts[2];
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
					vs_output* pclipped = ctxt->vso_pool->alloc();

					//LERP
					ctxt->vs_output_ops->lerp(*pclipped, *clipped_verts[src_stage][i], *clipped_verts[src_stage][j], d[0] / (d[0] - d[1]));

					clipped_verts[dest_stage][num_clipped_verts[dest_stage]] = pclipped;
					++ num_clipped_verts[dest_stage];
				}
			}
			else
			{
				if(d[1] >= 0.0f)
				{
					vs_output* pclipped = ctxt->vso_pool->alloc();

					//LERP
					ctxt->vs_output_ops->lerp(*pclipped, *clipped_verts[src_stage][j], *clipped_verts[src_stage][i], d[1] / (d[1] - d[0]));

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
				(*ctxt->is_front) = area > 0.0f;

				// If triangle is culled, return 0.
				if( ctxt->cull_fn(area) )
				{
					(*ctxt->num_clipped_verts) = 0;
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

	(*ctxt->num_clipped_verts) = num_final_clipped_verts;
	for(size_t i = 0; i < num_final_clipped_verts; ++i)
	{
		ctxt->clipped_verts[i] = clipped_verts[src_stage][i];
	}
}

END_NS_SALVIAR();
