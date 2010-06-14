#include "../include/clipper.h"

#include "../include/shaderregs_op.h"
#include "../include/shader.h"

#include <algorithm>
BEGIN_NS_SOFTART()


using namespace efl;
using namespace std;

clipper::clipper(){
	planes_[0] = vec4(1.0f, 0.0f, 0.0f, 1.0f);
	planes_[1] = vec4(0.0f, 1.0f, 0.0f, 1.0f);
	planes_[2] = vec4(0.0f, 0.0f, 1.0f, 0.0f);
	planes_[3] = vec4(-1.0f, 0.0f, 0.0f, 1.0f);
	planes_[4] = vec4(0.0f, -1.0f, 0.0f, 1.0f);
	planes_[5] = vec4(0.0f, 0.0f, -1.0f, 1.0f);

	std::fill(planes_enable_.begin(), planes_enable_.end(), true);
}

void clipper::set_clip_plane_enable(bool enable, size_t idx)
{
	if(idx >= plane_num){
		custom_assert(false, "");
	}

	planes_enable_[idx] = enable;
}

void clipper::clip(std::vector<vs_output> &out_clipped_verts, const viewport& vp, const vs_output& v0, const vs_output& v1) const
{
	efl::pool::stack_pool< vs_output, 12 > pool;
	std::vector<const vs_output*> clipped_verts[2];

	//开始clip, Ping-Pong idioms
	clipped_verts[0].push_back(&v0);
	clipped_verts[0].push_back(&v1);

	float di, dj;
	size_t src_stage = 0;
	size_t dest_stage = 1;

	for(size_t i_plane = 0; i_plane < planes_.size(); ++i_plane)
	{
		if( ! planes_enable_[i_plane] ){
			continue;
		}

		clipped_verts[dest_stage].clear();

		//for(int i = 0, j = 1; i < static_cast<int>(clipped_verts[src_stage].size()) - 1; ++i, ++j)
		if (!clipped_verts[src_stage].empty())
		{
			di = dot_prod4(planes_[i_plane], clipped_verts[src_stage][0]->position);
			dj = dot_prod4(planes_[i_plane], clipped_verts[src_stage][1]->position);

			if(di >= 0.0f){
				clipped_verts[dest_stage].resize(2);

				clipped_verts[dest_stage][0] = clipped_verts[src_stage][0];

				if(dj < 0.0f){
					vs_output* pclipped = (vs_output*)(pool.malloc());

					//LERP
					*pclipped = *clipped_verts[src_stage][0] + (*clipped_verts[src_stage][1] - *clipped_verts[src_stage][0]) * (di / (di - dj));
					clipped_verts[dest_stage][1] = pclipped;
				}
				else {
					clipped_verts[dest_stage][1] = clipped_verts[src_stage][1];
				}
			}
			else {
				if(dj >= 0.0f){
					clipped_verts[dest_stage].resize(2);

					vs_output* pclipped = (vs_output*)(pool.malloc());

					//LERP
					*pclipped = *clipped_verts[src_stage][1] + (*clipped_verts[src_stage][0] - *clipped_verts[src_stage][1]) * (dj / (dj - di));

					clipped_verts[dest_stage][0] = pclipped;

					clipped_verts[dest_stage][1] = clipped_verts[src_stage][1];
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
		viewport_transform(out_clipped_verts[i].position, vp);
	}
}

void clipper::clip(std::vector<vs_output> &out_clipped_verts, const viewport& vp, const vs_output& v0, const vs_output& v1, const vs_output& v2) const
{
	efl::pool::stack_pool< vs_output, 12 > pool;
	std::vector<const vs_output*> clipped_verts[2];

	//开始clip, Ping-Pong idioms
	clipped_verts[0].push_back(&v0);
	clipped_verts[0].push_back(&v1);
	clipped_verts[0].push_back(&v2);

	float d[2];
	size_t src_stage = 0;
	size_t dest_stage = 1;

	for(size_t i_plane = 0; i_plane < planes_.size(); ++i_plane)
	{
		if( ! planes_enable_[i_plane] ){
			continue;
		}

		clipped_verts[dest_stage].clear();

		if (!clipped_verts[src_stage].empty()){
			d[0] = dot_prod4(planes_[i_plane], clipped_verts[src_stage][0]->position);
		}
		for(size_t i = 0, j = 1; i < clipped_verts[src_stage].size(); ++i, ++j)
		{
			//wrap
			j %= clipped_verts[src_stage].size();

			d[1] = dot_prod4(planes_[i_plane], clipped_verts[src_stage][j]->position);

			if(d[0] >= 0.0f){
				clipped_verts[dest_stage].push_back(clipped_verts[src_stage][i]);

				if(d[1] < 0.0f){
					vs_output* pclipped = (vs_output*)(pool.malloc());

					//LERP
					*pclipped = *clipped_verts[src_stage][i] + (*clipped_verts[src_stage][j] - *clipped_verts[src_stage][i]) * (d[0] / (d[0] - d[1]));

					clipped_verts[dest_stage].push_back(pclipped);
				}
			}
			else {
				if(d[1] >= 0.0f){
					vs_output* pclipped = (vs_output*)(pool.malloc());

					//LERP
					*pclipped = *clipped_verts[src_stage][j] + (*clipped_verts[src_stage][i] - *clipped_verts[src_stage][j]) * (d[1] / (d[1] - d[0]));

					clipped_verts[dest_stage].push_back(pclipped);
				}
			}

			d[0] = d[1];
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
		viewport_transform(out_clipped_verts[i].position, vp);
	}
}
END_NS_SOFTART()
