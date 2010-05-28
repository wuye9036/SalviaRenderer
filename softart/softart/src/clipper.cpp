#include "../include/clipper.h"

#include "../include/shaderregs_op.h"
#include "../include/shader.h"

#include <algorithm>
BEGIN_NS_SOFTART()


using namespace efl;
using namespace std;

clipper::clipper(){
	//预先设置6个面，其余面清零
	planes_[0] = vec4(1.0f, 0.0f, 0.0f, 1.0f);
	planes_[1] = vec4(0.0f, 1.0f, 0.0f, 1.0f);
	planes_[2] = vec4(0.0f, 0.0f, 1.0f, 1.0f);
	planes_[3] = vec4(-1.0f, 0.0f, 0.0f, 1.0f);
	planes_[4] = vec4(0.0f, -1.0f, 0.0f, 1.0f);
	planes_[5] = vec4(0.0f, 0.0f, 0.0f, 1.0f);

	std::fill(planes_enable_.begin(), planes_enable_.begin()+default_plane_num, true);
	std::fill(planes_enable_.begin()+default_plane_num, planes_enable_.end(), false);
}


void clipper::set_clip_plane(const vec4& plane, size_t idx)
{
	if(idx >= default_plane_num + planes_.size()){
		custom_assert(false, "");
	}

	planes_[idx - default_plane_num] = plane;
}

void clipper::set_clip_plane_enable(bool /*enable*/, size_t idx)
{
	if(idx >= default_plane_num + planes_enable_.size()){
		custom_assert(false, "");
	}

	planes_enable_[idx - default_plane_num] = false;
}

void clipper::clip(std::vector<vs_output> &out_clipped_verts, const viewport& vp, const vs_output& v0, const vs_output& v1, const vs_output& v2) const
{
	efl::pool::stack_pool< vs_output, 20 > pool;
	std::vector<const vs_output*> clipped_verts[2];
	clipped_verts[0].clear();

	//开始clip, Ping-Pong idioms
	clipped_verts[0].push_back(&v0);
	clipped_verts[0].push_back(&v1);
	clipped_verts[0].push_back(&v2);

	float di, dj;
	size_t src_stage = 0;
	size_t dest_stage = 1;

	for(size_t i_plane = 0; i_plane < planes_.size(); ++i_plane)
	{
		if( ! planes_enable_[i_plane] ){
			continue;
		}

		clipped_verts[dest_stage].clear();

		for(size_t i = 0, j = 1; i < clipped_verts[src_stage].size(); ++i, ++j)
		{
			//wrap
			j %= clipped_verts[src_stage].size();

			di = dot_prod4(planes_[i_plane], clipped_verts[src_stage][i]->position);
			dj = dot_prod4(planes_[i_plane], clipped_verts[src_stage][j]->position);

			if(di >= 0.0f){
				clipped_verts[dest_stage].push_back(clipped_verts[src_stage][i]);

				if(dj < 0.0f){
					vs_output* pclipped = (vs_output*)(pool.malloc());

					//LERP
					*pclipped = *clipped_verts[src_stage][i] + (*clipped_verts[src_stage][j] - *clipped_verts[src_stage][i]) * ( di / (di - dj));
					update_wpos(*pclipped, vp);

					clipped_verts[dest_stage].push_back(pclipped);
				}
			} else {
				if(dj >= 0.0f){
					vs_output* pclipped = (vs_output*)(pool.malloc());

					//LERP
					*pclipped = *clipped_verts[src_stage][j] + (*clipped_verts[src_stage][i] - *clipped_verts[src_stage][j]) * ( dj / (dj - di));
					update_wpos(*pclipped, vp);

					clipped_verts[dest_stage].push_back(pclipped);
				}
			}
		}

		//swap src and dest pool
		++src_stage;
		++dest_stage;
		src_stage &= 1;
		dest_stage &= 1;
	}
	//返回
	clipped_verts[dest_stage].clear();

	const std::vector<const vs_output*> &clipped_verts_ptrs = clipped_verts[src_stage];
	out_clipped_verts.resize(clipped_verts_ptrs.size());
	for(size_t i = 0; i < clipped_verts_ptrs.size(); ++i){
		out_clipped_verts[i] = *clipped_verts_ptrs[i];
	}
}
END_NS_SOFTART()
