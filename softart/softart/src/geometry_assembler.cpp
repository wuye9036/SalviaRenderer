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

	h_rasterizer hrast = pparent_->get_rasterizer();
	switch(primtopo_)
	{
	case primitive_line_list:
		for(size_t iprim = 0; iprim < prim_count; ++iprim){
			hrast->rasterize_line(dvc_.fetch(iprim*2), dvc_.fetch(iprim*2+1));
		}
		break;
	case primitive_line_strip:
		for(size_t iprim = 0; iprim < prim_count; ++iprim){
			hrast->rasterize_line(dvc_.fetch(iprim), dvc_.fetch(iprim+1));
		}
		break;
	case primitive_triangle_list:
		for(size_t iprim = 0; iprim < prim_count; ++iprim){
			hrast->rasterize_triangle(dvc_.fetch(iprim*3), dvc_.fetch(iprim*3+1), dvc_.fetch(iprim*3+2));
		}
		break;
	case primitive_triangle_strip:
		for(size_t iprim = 0; iprim < prim_count; ++iprim){
			if(iprim % 2 == 0){
				hrast->rasterize_triangle(dvc_.fetch(iprim), dvc_.fetch(iprim+1), dvc_.fetch(iprim+2));
			} else {
				hrast->rasterize_triangle(dvc_.fetch(iprim+2), dvc_.fetch(iprim+1), dvc_.fetch(iprim));
			}
		}
		break;
	default:
		custom_assert(false, "Primitive Topology错了！");
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
	h_rasterizer hrast = pparent_->get_rasterizer();
	if(!hrast) return;

	switch(primtopo_)
	{
	case primitive_line_list:
		for(size_t iprim = 0; iprim < prim_count; ++iprim){
			hrast->rasterize_line(
				dvc_.fetch(pidx[iprim*2]), dvc_.fetch(pidx[iprim*2+1]));
		}
		break;
	case primitive_line_strip:
		for(size_t iprim = 0; iprim < prim_count; ++iprim){
			hrast->rasterize_line(dvc_.fetch(pidx[iprim]), dvc_.fetch(pidx[iprim+1]));
		}
		break;
	case primitive_triangle_list:
		for(size_t iprim = 0; iprim < prim_count; ++iprim){
			hrast->rasterize_triangle(dvc_.fetch(pidx[iprim*3]), dvc_.fetch(pidx[iprim*3+1]), dvc_.fetch(pidx[iprim*3+2]));
		}
		break;
	case primitive_triangle_strip:
		for(size_t iprim = 0; iprim < prim_count; ++iprim){
			//TODO: 需要支持index restart, DX 10 Spec

			if(iprim % 2 == 0){
				hrast->rasterize_triangle(dvc_.fetch(pidx[iprim]), dvc_.fetch(pidx[iprim+1]), dvc_.fetch(pidx[iprim+2]));
			} else {
				hrast->rasterize_triangle(dvc_.fetch(pidx[iprim+2]), dvc_.fetch(pidx[iprim+1]), dvc_.fetch(pidx[iprim]));
			}
		}
		break;
	default:
		custom_assert(false, "无效的Primitive Topology");
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