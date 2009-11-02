#include "../include/clipper.h"

#include "../include/shaderregs_op.h"
#include "../include/shader.h"

#include <algorithm>

using namespace efl;
using namespace std;

clipper::clipper():last_stage(0), pool_(sizeof(vs_output_impl)){
	//预先设置6个面，其余面清零
	planes_[0] = vec4(1.0f, 0.0f, 0.0f, 1.0f);
	planes_[1] = vec4(0.0f, 1.0f, 0.0f, 1.0f);
	planes_[2] = vec4(0.0f, 0.0f, 1.0f, 1.0f);
	planes_[3] = vec4(-1.0f, 0.0f, 0.0f, 1.0f);
	planes_[4] = vec4(0.0f, -1.0f, 0.0f, 1.0f);
	planes_[5] = vec4(0.0f, 0.0f, -1.0f, 1.0f);

	std::fill(planes_enable_.begin(), planes_enable_.begin()+6, true);
	std::fill(planes_enable_.begin()+6, planes_enable_.end(), false);
}

void clipper::set_viewport(const viewport& vp)
{
	pvp_ = &vp;
}

void clipper::set_clip_plane(const vec4& plane, size_t idx)
{
	if(idx >= 6 + planes_.size()){
		custom_assert(false, "");
	}

	planes_[idx - 6] = plane;
}

void clipper::set_clip_plane_enable(bool /*enable*/, size_t idx)
{
	if(idx >= 6 + planes_enable_.size()){
		custom_assert(false, "");
	}

	planes_enable_[idx - 6] = false;
}

const vector<const vs_output_impl*>& clipper::clip(const vs_output_impl& v0, const vs_output_impl& v1, const vs_output_impl& v2)
{
	//清理上一次clipper所使用的内存
	for(size_t i = 0; i < 2; ++i)
	{
		for(size_t ivert = 0; ivert < clipped_verts_[i].size(); ++ivert)	{
			if(is_vert_from_pool_[i][ivert]){
				pool_.free(const_cast<vs_output_impl*>(clipped_verts_[i][ivert]));
			}
		}
	}

	is_vert_from_pool_[0].clear();
	clipped_verts_[0].clear();

	//开始clip, Ping-Pong idioms
	clipped_verts_[0].push_back(&v0);
	clipped_verts_[0].push_back(&v1);
	clipped_verts_[0].push_back(&v2);

	is_vert_from_pool_[0].push_back(false);
	is_vert_from_pool_[0].push_back(false);
	is_vert_from_pool_[0].push_back(false);

	float di, dj;
	size_t src_stage = 0;
	size_t dest_stage = 1;

	for(size_t i_plane = 0; i_plane < planes_.size(); ++i_plane)
	{
		if( ! planes_enable_[i_plane] ){
			continue;
		}

		clipped_verts_[dest_stage].clear();
		is_vert_from_pool_[dest_stage].clear();

		for(size_t i = 0, j = 1; i < clipped_verts_[src_stage].size(); ++i, ++j)
		{
			//wrap
			j %= clipped_verts_[src_stage].size();

			di = dot_prod4(planes_[i_plane], clipped_verts_[src_stage][i]->position);
			dj = dot_prod4(planes_[i_plane], clipped_verts_[src_stage][j]->position);

			if(di >= 0.0f){
				clipped_verts_[dest_stage].push_back(clipped_verts_[src_stage][i]);
				is_vert_from_pool_[dest_stage].push_back(false);

				if(dj < 0.0f){
					vs_output_impl* pclipped = (vs_output_impl*)(pool_.malloc());

					//LERP
					*pclipped = *clipped_verts_[src_stage][i] + (*clipped_verts_[src_stage][j] - *clipped_verts_[src_stage][i]) * ( di / (di - dj));
					update_wpos(*pclipped, *pvp_);

					clipped_verts_[dest_stage].push_back(pclipped);
					is_vert_from_pool_[dest_stage].push_back(true);
				}
			} else {
				if(dj >= 0.0f){
					vs_output_impl* pclipped = (vs_output_impl*)(pool_.malloc());

					//LERP
					*pclipped = *clipped_verts_[src_stage][j] + (*clipped_verts_[src_stage][i] - *clipped_verts_[src_stage][j]) * ( dj / (dj - di));
					update_wpos(*pclipped, *pvp_);

					clipped_verts_[dest_stage].push_back(pclipped);
					is_vert_from_pool_[dest_stage].push_back(true);
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
	clipped_verts_[dest_stage].clear();
	is_vert_from_pool_[dest_stage].clear();
	return clipped_verts_[src_stage];
}