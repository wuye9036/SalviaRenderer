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
#include "../include/cpuinfo.h"

#include "../include/thread_pool.h"

#include <boost/bind.hpp>
BEGIN_NS_SOFTART()


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
	pixel_shader *pps = pparent_->get_pixel_shader().get();
	switch(primtopo_)
	{
	case primitive_line_list:
		for(size_t iprim = 0; iprim < prim_count; ++iprim){
			hrast->rasterize_line(dvc_.fetch(iprim*2), dvc_.fetch(iprim*2+1), vp, pps);
		}
		break;
	case primitive_line_strip:
		for(size_t iprim = 0; iprim < prim_count; ++iprim){
			hrast->rasterize_line(dvc_.fetch(iprim), dvc_.fetch(iprim+1), vp, pps);
		}
		break;
	case primitive_triangle_list:
		for(size_t iprim = 0; iprim < prim_count; ++iprim){
			hrast->rasterize_triangle(dvc_.fetch(iprim*3), dvc_.fetch(iprim*3+1), dvc_.fetch(iprim*3+2), vp, pps);
		}
		break;
	case primitive_triangle_strip:
		for(size_t iprim = 0; iprim < prim_count; ++iprim){
			if(iprim % 2 == 0){
				hrast->rasterize_triangle(dvc_.fetch(iprim), dvc_.fetch(iprim+1), dvc_.fetch(iprim+2), vp, pps);
			} else {
				hrast->rasterize_triangle(dvc_.fetch(iprim+2), dvc_.fetch(iprim+1), dvc_.fetch(iprim), vp, pps);
			}
		}
		break;
	default:
		custom_assert(false, "Primitive Topology错了！");
	}
}

template<class T>
void geometry_assembler::dispatch_primitive_impl(std::vector<lockfree_queue<uint32_t> >& tiles, std::vector<T> const & indices, atomic<int32_t>& working_prim, int32_t prim_count){
	//TODO: 需要支持index restart, DX 10 Spec

	int32_t local_working_prim = working_prim ++;

	int N = 0;
	switch(primtopo_)
	{
	case primitive_line_list:
	case primitive_line_strip:
		N = 2;
		break;
	case primitive_triangle_list:
	case primitive_triangle_strip:
		N = 3;
		break;
	default:
		custom_assert(false, "geometry_assembler::dispatch_primitive_impl not support this primitive type.");

		break;
	}

	float x_min;
	float x_max;
	float y_min;
	float y_max;
	while (local_working_prim < prim_count)
	{
		const vs_output& v0 = dvc_.fetch(indices[local_working_prim * N + 0]);
		x_min = v0.wpos.x;
		x_max = v0.wpos.x;
		y_min = v0.wpos.y;
		y_max = v0.wpos.y;

		for (size_t i = 1; i < N; ++ i)
		{
			const vs_output& v = dvc_.fetch(indices[local_working_prim * N + i]);
			x_min = min(x_min, v.wpos.x);
			x_max = max(x_max, v.wpos.x);
			y_min = min(y_min, v.wpos.y);
			y_max = max(y_max, v.wpos.y);
		}

		int sx = efl::clamp(static_cast<int>(floor(x_min / TILE_SIZE)), 0, num_tiles_x_);
		int sy = efl::clamp(static_cast<int>(floor(y_min / TILE_SIZE)), 0, num_tiles_y_);
		int ex = efl::clamp(static_cast<int>(ceil(x_max / TILE_SIZE)), 0, num_tiles_x_);
		int ey = efl::clamp(static_cast<int>(ceil(y_max / TILE_SIZE)), 0, num_tiles_y_);

		for (int y = sy; y < ey; ++ y){
			for (int x = sx; x < ex; ++ x){
				lockfree_queue<uint32_t>& tile = tiles[y * num_tiles_x_ + x];
				tile.enqueue(local_working_prim);
			}
		}

		local_working_prim = working_prim ++;
	}
}

template<class T>
void geometry_assembler::rasterize_primitive_func(std::vector<lockfree_queue<uint32_t> >& tiles, const std::vector<T>& indices, atomic<int32_t>& working_tile , pixel_shader *pps)
{
	const h_rasterizer& hrast = pparent_->get_rasterizer();
	const viewport& vp = pparent_->get_viewport();

	viewport tile_vp;
	tile_vp.w = TILE_SIZE;
	tile_vp.h = TILE_SIZE;
	tile_vp.minz = vp.minz;
	tile_vp.maxz = vp.maxz;

	int32_t local_working_tile = working_tile ++;

	while (local_working_tile < tiles.size()){
		lockfree_queue<uint32_t>& prims = tiles[local_working_tile];
		int y = local_working_tile / num_tiles_x_;
		int x = local_working_tile - y * num_tiles_x_;
		tile_vp.x = x * TILE_SIZE;
		tile_vp.y = y * TILE_SIZE;

		uint32_t iprim;
		switch(primtopo_){
		case primitive_line_list:
		case primitive_line_strip:
			while (!prims.empty()){
				prims.dequeue(iprim);
				hrast->rasterize_line(dvc_.fetch(indices[iprim * 2 + 0]), dvc_.fetch(indices[iprim * 2 + 1]), tile_vp, pps);
			}
			break;
		case primitive_triangle_list:
		case primitive_triangle_strip:
			while (!prims.empty()){
				prims.dequeue(iprim);
				hrast->rasterize_triangle(dvc_.fetch(indices[iprim * 3 + 0]), dvc_.fetch(indices[iprim * 3 + 1]), dvc_.fetch(indices[iprim * 3 + 2]), tile_vp, pps);
			}
			break;
		}

		local_working_tile = working_tile ++;
	}
}

template<class T>
void geometry_assembler::generate_line_list_indices_impl(std::vector<T>& indices, T* pidx, atomic<int32_t>& working_prim, int32_t prim_count)
{
	int32_t local_working_prim = working_prim ++;

	while (local_working_prim < prim_count){
		indices[local_working_prim * 2 + 0] = pidx[local_working_prim * 2 + 0];
		indices[local_working_prim * 2 + 1] = pidx[local_working_prim * 2 + 1];

		local_working_prim = working_prim ++;
	}
}

template<class T>
void geometry_assembler::generate_line_strip_indices_impl(std::vector<T>& indices, T* pidx, atomic<int32_t>& working_prim, int32_t prim_count)
{
	int32_t local_working_prim = working_prim ++;

	while (local_working_prim < prim_count){
		indices[local_working_prim * 2 + 0] = pidx[local_working_prim + 0];
		indices[local_working_prim * 2 + 1] = pidx[local_working_prim + 1];

		local_working_prim = working_prim ++;
	}
}

template<class T>
void geometry_assembler::generate_triangle_list_indices_impl(std::vector<T>& indices, T* pidx, atomic<int32_t>& working_prim, int32_t prim_count)
{
	int32_t local_working_prim = working_prim ++;

	while (local_working_prim < prim_count){
		indices[local_working_prim * 3 + 0] = pidx[local_working_prim * 3 + 0];
		indices[local_working_prim * 3 + 1] = pidx[local_working_prim * 3 + 1];
		indices[local_working_prim * 3 + 2] = pidx[local_working_prim * 3 + 2];

		local_working_prim = working_prim ++;
	}
}

template<class T>
void geometry_assembler::generate_triangle_strip_indices_impl(std::vector<T>& indices, T* pidx, atomic<int32_t>& working_prim, int32_t prim_count)
{
	int32_t local_working_prim = working_prim ++;

	while (local_working_prim < prim_count){
		indices[local_working_prim * 3 + 0] = pidx[local_working_prim + 0];
		indices[local_working_prim * 3 + 1] = pidx[local_working_prim + 1];
		indices[local_working_prim * 3 + 2] = pidx[local_working_prim + 2];

		if(local_working_prim & 1){
			std::swap(indices[local_working_prim * 3 + 0], indices[local_working_prim * 3 + 2]);
		}

		local_working_prim = working_prim ++;
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
	std::vector<lockfree_queue<uint32_t> > tiles(num_tiles_x_ * num_tiles_y_);

	std::vector<T> indices;
	switch(primtopo_)
	{
	case primitive_line_list:
	case primitive_line_strip:
		indices.resize(prim_count * 2);
		break;

	case primitive_triangle_list:
	case primitive_triangle_strip:
		indices.resize(prim_count * 3);
		break;
	}

	atomic<int32_t> working_prim(0);
#ifdef SOFTART_MULTITHEADING_ENABLED

	for (size_t i = 0; i < num_cpu_cores(); ++ i){
		switch(primtopo_)
		{
		case primitive_line_list:
			global_thread_pool().schedule(boost::bind(&geometry_assembler::generate_line_list_indices_impl<T>, this, boost::ref(indices), pidx, boost::ref(working_prim), prim_count));
			break;
	
		case primitive_line_strip:
			global_thread_pool().schedule(boost::bind(&geometry_assembler::generate_line_list_indices_impl<T>, this, boost::ref(indices), pidx, boost::ref(working_prim), prim_count));
			break;
		
		case primitive_triangle_list:
			global_thread_pool().schedule(boost::bind(&geometry_assembler::generate_triangle_list_indices_impl<T>, this, boost::ref(indices), pidx, boost::ref(working_prim), prim_count));
			break;

		case primitive_triangle_strip:
			global_thread_pool().schedule(boost::bind(&geometry_assembler::generate_triangle_strip_indices_impl<T>, this, boost::ref(indices), pidx, boost::ref(working_prim), prim_count));
			break;
		}
	}
	global_thread_pool().wait();
#else
	switch(primtopo_)
	{
	case primitive_line_list:
		geometry_assembler::generate_line_list_indices_impl<T>(boost::ref(indices), pidx, boost::ref(working_prim), prim_count);
		break;

	case primitive_line_strip:
		geometry_assembler::generate_line_list_indices_impl<T>(boost::ref(indices), pidx, boost::ref(working_prim), prim_count);
		break;

	case primitive_triangle_list:
		geometry_assembler::generate_triangle_list_indices_impl<T>(boost::ref(indices), pidx, boost::ref(working_prim), prim_count);
		break;

	case primitive_triangle_strip:
		geometry_assembler::generate_triangle_strip_indices_impl<T>(boost::ref(indices), pidx, boost::ref(working_prim), prim_count);
		break;
	}
#endif

	dvc_.transform_vertices(indices);

	working_prim = 0;
#ifdef SOFTART_MULTITHEADING_ENABLED
	for (size_t i = 0; i < num_cpu_cores(); ++ i)
	{
		global_thread_pool().schedule(boost::bind(&geometry_assembler::dispatch_primitive_impl<T>, this, boost::ref(tiles), boost::ref(indices), boost::ref(working_prim), prim_count));
	}
	global_thread_pool().wait();
#else
	geometry_assembler::dispatch_primitive_impl<T>(boost::ref(tiles), boost::ref(indices), boost::ref(working_prim), prim_count);
#endif



	atomic<int32_t> working_tile(0);
	h_pixel_shader hps = pparent_->get_pixel_shader();
#ifdef SOFTART_MULTITHEADING_ENABLED
	size_t num_threads = num_cpu_cores();
	pixel_shader **ppps = new pixel_shader*[num_threads];
	for (size_t i = 0; i < num_threads; ++ i)
	{
		// create pixel_shader clone per thread from hps
		ppps[i] = hps->create_clone();
		global_thread_pool().schedule(boost::bind(&geometry_assembler::rasterize_primitive_func<T>, this, boost::ref(tiles), boost::ref(indices), boost::ref(working_tile), ppps[i]));
	}
	global_thread_pool().wait();
	// destroy all pixel_shader clone
	for (size_t i = 0; i < num_threads; ++ i)
	{
		hps->destroy_clone(ppps[i]);
	}
	delete[] ppps;
#else
	geometry_assembler::rasterize_primitive_func<T>(boost::ref(tiles), boost::ref(indices), boost::ref(working_tile), hps.get());
#endif
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

END_NS_SOFTART()
