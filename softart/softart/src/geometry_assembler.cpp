#include "../include/geometry_assembler.h"

#include "eflib/include/eflib.h"

#include "../include/shaderregs_op.h"
#include "../include/shader.h"
#include "../include/renderer_impl.h"

#include "../include/stream.h"
#include "../include/buffer.h"
#include "../include/rasterizer.h"
#include "../include/stream_assembler.h"
#include "../include/vertex_cache.h"

using namespace std;
using namespace efl;

const int TILE_SIZE = 64;

template<class T>
void geometry_assembler::get_vert_range(size_t startpos, size_t count, int basevert, size_t& min_vert, size_t& max_vert)
{
	min_vert = 1;
	max_vert = 0;

	custom_assert(indexbuf_, "使用了空的索引缓存！");
	custom_assert((startpos + count - 1) * sizeof(T) <= indexbuf_->get_size(), "索引使用越界！");

	if(! indexbuf_) return;
	if(! ((startpos + count) * sizeof(T) <= indexbuf_->get_size())) return;

	T* pidx = (T*)(indexbuf_->raw_data(startpos * sizeof(T)));
	T minv = *pidx;
	T maxv = *pidx;
	for(size_t i_idx = 0; i_idx < count; ++i_idx){
		if(*pidx < minv){
			minv = *pidx;
		} else {
			if(*pidx > maxv){
				maxv = *pidx;
			}
		}
		++pidx;
	}

	min_vert = size_t((int)minv + (int)basevert);
	max_vert = size_t((int)maxv + (int)basevert);
}

vs_output* geometry_assembler::transform_vertex(size_t startpos, size_t count)
{
	//进行顶点变换
	dvc_.set_vert_range(startpos, startpos+count);
	dvc_.resize(count);

	return NULL;
}

void geometry_assembler::initialize(renderer_impl* pparent)
{
	custom_assert(pparent, "初始化出现异常，传入的初始化指针为空！");

	pparent_ = pparent;
	dvc_.initialize(pparent);
}

geometry_assembler::geometry_assembler()
:indexbuf_((buffer*)NULL),
primtopo_(primitive_triangle_list), idxtype_(index_int16)
{}

void geometry_assembler::set_primitive_topology(primitive_topology prim_topo){
	switch (prim_topo)
	{
	case primitive_line_list:
	case primitive_line_strip:
	case primitive_triangle_list:
	case primitive_triangle_strip:
		break;
	default:
		custom_assert(false, "枚举值无效：无效的图元拓扑枚举。");
		return;
	}

	primtopo_ = prim_topo;
}

void geometry_assembler::set_stream(size_t index, h_buffer hbuf){
	sa_.set_stream(stream_index(index), hbuf);
}

void geometry_assembler::set_index_buffer(h_buffer hbuf, index_type idxtype){
	switch (idxtype)
	{
	case index_int16:
	case index_int32:
		break;
	default:
		custom_assert(false, "枚举值无效：无效的索引类型");
		return;
	}

	indexbuf_ = hbuf;
	idxtype_ = idxtype;
}

void geometry_assembler::set_input_layout(const vector<input_element_decl>& elem_decl){
	//layout_ 只能到运行期检测了...
	sa_.set_input_layout(elem_decl);
}

void geometry_assembler::draw(size_t startpos, size_t prim_count){

	dvc_.reset();

	custom_assert(pparent_, "Renderer 指针为空！可能该对象没有经过正确的初始化！");
	if(!pparent_) return;

	size_t vert_count = 0;

	switch(primtopo_)
	{
	case primitive_line_list:
		vert_count = prim_count * 2;
		break;
	case primitive_line_strip:
		vert_count = prim_count + 1;
		break;
	case primitive_triangle_list:
		vert_count = prim_count * 3;
		break;
	case primitive_triangle_strip:
		vert_count = prim_count + 2;
		break;
	default:
		custom_assert(false, "Primitive Topology错了！");
	}

	transform_vertex(startpos, vert_count);

	const viewport& vp = pparent_->get_viewport();
	const h_rasterizer& hrast = pparent_->get_rasterizer();
	switch(primtopo_)
	{
	case primitive_line_list:
		for(size_t iprim = 0; iprim < prim_count; ++iprim){
			hrast->rasterize_line(dvc_.fetch(iprim*2), dvc_.fetch(iprim*2+1), vp);
		}
		break;
	case primitive_line_strip:
		for(size_t iprim = 0; iprim < prim_count; ++iprim){
			hrast->rasterize_line(dvc_.fetch(iprim), dvc_.fetch(iprim+1), vp);
		}
		break;
	case primitive_triangle_list:
		for(size_t iprim = 0; iprim < prim_count; ++iprim){
			hrast->rasterize_triangle(dvc_.fetch(iprim*3), dvc_.fetch(iprim*3+1), dvc_.fetch(iprim*3+2), vp);
		}
		break;
	case primitive_triangle_strip:
		for(size_t iprim = 0; iprim < prim_count; ++iprim){
			if(iprim % 2 == 0){
				hrast->rasterize_triangle(dvc_.fetch(iprim), dvc_.fetch(iprim+1), dvc_.fetch(iprim+2), vp);
			} else {
				hrast->rasterize_triangle(dvc_.fetch(iprim+2), dvc_.fetch(iprim+1), dvc_.fetch(iprim), vp);
			}
		}
		break;
	default:
		custom_assert(false, "Primitive Topology错了！");
	}
}

template<class T, int N>
void geometry_assembler::dispatch_primitive_impl(std::vector<std::vector<T> >& tiles, T* indices){
	//TODO: 需要支持index restart, DX 10 Spec

	const vs_output& v0 = dvc_.fetch(indices[0]);
	float x_min = v0.wpos.x;
	float x_max = v0.wpos.x;
	float y_min = v0.wpos.y;
	float y_max = v0.wpos.y;

	for (int i = 1; i < N; ++ i)
	{
		const vs_output& v = dvc_.fetch(indices[i]);
		x_min = std::min(x_min, v.wpos.x);
		x_max = std::max(x_max, v.wpos.x);
		y_min = std::min(y_min, v.wpos.y);
		y_max = std::max(y_max, v.wpos.y);
	}

	int sx = efl::clamp(static_cast<int>(floor(x_min / TILE_SIZE)), 0, num_tiles_x_);
	int sy = efl::clamp(static_cast<int>(floor(y_min / TILE_SIZE)), 0, num_tiles_y_);
	int ex = efl::clamp(static_cast<int>(ceil(x_max / TILE_SIZE)), 0, num_tiles_x_);
	int ey = efl::clamp(static_cast<int>(ceil(y_max / TILE_SIZE)), 0, num_tiles_y_);

	for (int y = sy; y < ey; ++ y){
		for (int x = sx; x < ex; ++ x){
			std::vector<T>& tile = tiles[y * num_tiles_x_ + x];
			tile.insert(tile.end(), indices, indices + N);
		}
	}
}

template<class T>
void geometry_assembler::draw_index_impl(size_t startpos, size_t prim_count, int basevert){

	custom_assert(pparent_, "Renderer 指针为空！可能该对象没有经过正确的初始化！");
	if(!pparent_) return;

	//计算索引数量
	size_t index_count = 0;
	switch(primtopo_)
	{
	case primitive_line_list:
		index_count = prim_count * 2;
		break;
	case primitive_line_strip:
		index_count = prim_count + 1;
		break;
	case primitive_triangle_list:
		index_count = prim_count * 3;
		break;
	case primitive_triangle_strip:
		index_count = prim_count + 2;
		break;
	default:
		custom_assert(false, "枚举值无效：无效的Primitive Topology");
		return;
	}

	//统计顶点区间
	size_t min_vert, max_vert;
	get_vert_range<T>(startpos, index_count, basevert, min_vert, max_vert);

	if(min_vert > max_vert) return;

	//变换顶点
	transform_vertex(min_vert, max_vert - min_vert + 1);
	T* pidx = (T*)(indexbuf_->raw_data(startpos * sizeof(T)));

	//组织索引并光栅化
	const h_rasterizer& hrast = pparent_->get_rasterizer();
	if(!hrast) return;

	const viewport& vp = pparent_->get_viewport();
	num_tiles_x_ = static_cast<size_t>(vp.w + TILE_SIZE - 1) / TILE_SIZE;
	num_tiles_y_ = static_cast<size_t>(vp.h + TILE_SIZE - 1) / TILE_SIZE;
	std::vector<std::vector<T> > tiles(num_tiles_x_ * num_tiles_y_);

	switch(primtopo_)
	{
	case primitive_line_list:
		for(size_t iprim = 0; iprim < prim_count; ++iprim){
			T indices[2] = { pidx[iprim * 2 + 0], pidx[iprim * 2 + 1] };
			dispatch_primitive_impl<T, 2>(tiles, indices);
		}
		break;
	case primitive_line_strip:
		for(size_t iprim = 0; iprim < prim_count; ++iprim){
			T indices[2] = { pidx[iprim + 0], pidx[iprim + 1] };
			dispatch_primitive_impl<T, 2>(tiles, indices);
		}
		break;
	case primitive_triangle_list:
		for(size_t iprim = 0; iprim < prim_count; ++iprim){
			T indices[3] = { pidx[iprim * 3 + 0], pidx[iprim * 3 + 1], pidx[iprim * 3 + 2] };
			dispatch_primitive_impl<T, 3>(tiles, indices);
		}
		break;
	case primitive_triangle_strip:
		for(size_t iprim = 0; iprim < prim_count; ++iprim){
			T indices[3] = { pidx[iprim + 0], pidx[iprim + 1], pidx[iprim + 2] };
			if(iprim & 1){
				std::swap(indices[0], indices[2]);
			}

			dispatch_primitive_impl<T, 3>(tiles, indices);
		}
		break;
	}

	viewport tile_vp;
	tile_vp.w = TILE_SIZE;
	tile_vp.h = TILE_SIZE;
	tile_vp.minz = vp.minz;
	tile_vp.maxz = vp.maxz;
	for (int y = 0; y < num_tiles_y_; ++ y){
		for (int x = 0; x < num_tiles_x_; ++ x){
			const std::vector<T>& prims = tiles[y * num_tiles_x_ + x];
			tile_vp.x = x * TILE_SIZE;
			tile_vp.y = y * TILE_SIZE;

			switch(primtopo_){
			case primitive_line_list:
			case primitive_line_strip:
				for(size_t iprim = 0; iprim < prims.size(); iprim += 2){
					hrast->rasterize_line(dvc_.fetch(prims[iprim + 0]), dvc_.fetch(prims[iprim + 0]), tile_vp);
				}
				break;
			case primitive_triangle_list:
			case primitive_triangle_strip:
				for(size_t iprim = 0; iprim < prims.size(); iprim += 3){
					hrast->rasterize_triangle(dvc_.fetch(prims[iprim + 0]), dvc_.fetch(prims[iprim + 1]), dvc_.fetch(prims[iprim + 2]), tile_vp);
				}
				break;
			}
		}
	}
}

void geometry_assembler::draw_index(size_t startpos, size_t prim_count, int basevert){
	dvc_.reset();
	if(idxtype_ == index_int16){
		draw_index_impl<uint16_t>(startpos, prim_count, basevert);
		return;
	}
	if(idxtype_ == index_int32){
		draw_index_impl<uint32_t>(startpos, prim_count, basevert);
		return;		
	}
}