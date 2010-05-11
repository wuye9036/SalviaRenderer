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

#include <iterator>

#include <boost/bind.hpp>
BEGIN_NS_SOFTART()


using namespace std;
using namespace efl;

const int TILE_SIZE = 64;
const int DISPATCH_PRIMITIVE_PACKAGE_SIZE = 1;
const int RASTERIZE_PRIMITIVE_PACKAGE_SIZE = 1;

void geometry_assembler::initialize(renderer_impl* pparent)
{
	custom_assert(pparent, "初始化出现异常，传入的初始化指针为空！");

	pparent_ = pparent;
}

geometry_assembler::geometry_assembler()
{}

void geometry_assembler::dispatch_primitive_func(std::vector<lockfree_queue<uint32_t> >& tiles, int32_t prim_count, uint32_t stride, atomic<int32_t>& working_package, int32_t package_size){
	h_vertex_cache dvc = pparent_->get_vertex_cache();

	const int32_t num_packages = (prim_count + package_size - 1) / package_size;
	
	float x_min;
	float x_max;
	float y_min;
	float y_max;
	int32_t local_working_package = working_package ++;
	while (local_working_package < num_packages)
	{
		const int32_t start = local_working_package * package_size;
		const int32_t end = min(prim_count, start + package_size);
		for (int32_t i = start; i < end; ++ i)
		{
			{
				const vs_output& v0 = dvc->fetch(i * stride + 0);
				const float sign_w = sign(v0.wpos.w);
				const float x = v0.wpos.x * sign_w;
				const float y = v0.wpos.y * sign_w;
				x_min = x;
				x_max = x;
				y_min = y;
				y_max = y;
			}
			for (size_t j = 1; j < stride; ++ j)
			{
				const vs_output& v = dvc->fetch(i * stride + j);
				const float sign_w = sign(v.wpos.w);
				const float x = v.wpos.x * sign_w;
				const float y = v.wpos.y * sign_w;
				x_min = min(x_min, x);
				x_max = max(x_max, x);
				y_min = min(y_min, y);
				y_max = max(y_max, y);
			}

			const int sx = min(fast_floori(max(0, x_min) / TILE_SIZE), num_tiles_x_);
			const int sy = min(fast_floori(max(0, y_min) / TILE_SIZE), num_tiles_y_);
			const int ex = min(fast_ceili(max(0, x_max) / TILE_SIZE) + 1, num_tiles_x_);
			const int ey = min(fast_ceili(max(0, y_max) / TILE_SIZE) + 1, num_tiles_y_);
			for (int y = sy; y < ey; ++ y){
				for (int x = sx; x < ex; ++ x){
					lockfree_queue<uint32_t>& tile = tiles[y * num_tiles_x_ + x];
					tile.enqueue(i);
				}
			}
		}

		local_working_package = working_package ++;
	}
}

void geometry_assembler::rasterize_primitive_func(std::vector<lockfree_queue<uint32_t> >& tiles, const h_pixel_shader& pps, atomic<int32_t>& working_package, int32_t package_size)
{
	h_vertex_cache dvc = pparent_->get_vertex_cache();
	primitive_topology primtopo = pparent_->get_primitive_topology();

	const int32_t num_tiles = static_cast<int32_t>(tiles.size());
	const int32_t num_packages = (num_tiles + package_size - 1) / package_size;

	const h_rasterizer& hrast = pparent_->get_rasterizer();
	const viewport& vp = pparent_->get_viewport();

	viewport tile_vp;
	tile_vp.w = TILE_SIZE;
	tile_vp.h = TILE_SIZE;
	tile_vp.minz = vp.minz;
	tile_vp.maxz = vp.maxz;

	int32_t local_working_package = working_package ++;
	while (local_working_package < num_packages){
		const int32_t start = local_working_package * package_size;
		const int32_t end = min(num_tiles, start + package_size);
		for (int32_t i = start; i < end; ++ i)
		{
			lockfree_queue<uint32_t>& prims = tiles[i];

			std::vector<uint32_t> sorted_prims;
			prims.dequeue_all(std::back_insert_iterator<std::vector<uint32_t> >(sorted_prims));
			std::sort(sorted_prims.begin(), sorted_prims.end());

			int y = i / num_tiles_x_;
			int x = i - y * num_tiles_x_;
			tile_vp.x = static_cast<float>(x * TILE_SIZE);
			tile_vp.y = static_cast<float>(y * TILE_SIZE);

			uint32_t iprim;
			switch(primtopo){
			case primitive_line_list:
			case primitive_line_strip:
				for (size_t i = 0; i < sorted_prims.size(); ++ i){
					iprim = sorted_prims[i];
					hrast->rasterize_line(dvc->fetch(iprim * 2 + 0), dvc->fetch(iprim * 2 + 1), tile_vp, pps);
				}
				break;
			case primitive_triangle_list:
			case primitive_triangle_strip:
				for (size_t i = 0; i < sorted_prims.size(); ++ i){
					iprim = sorted_prims[i];
					hrast->rasterize_triangle(dvc->fetch(iprim * 3 + 0), dvc->fetch(iprim * 3 + 1), dvc->fetch(iprim * 3 + 2), tile_vp, pps);
				}
				break;
			}
		}

		local_working_package = working_package ++;
	}
}

void geometry_assembler::draw(size_t prim_count){

	custom_assert(pparent_, "Renderer 指针为空！可能该对象没有经过正确的初始化！");
	if(!pparent_) return;

	primitive_topology primtopo = pparent_->get_primitive_topology();

	//计算索引数量
	size_t index_count = 0;
	switch(primtopo)
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

	//组织索引并光栅化
	const h_rasterizer& hrast = pparent_->get_rasterizer();
	if(!hrast) return;

	const viewport& vp = pparent_->get_viewport();
	num_tiles_x_ = static_cast<size_t>(vp.w + TILE_SIZE - 1) / TILE_SIZE;
	num_tiles_y_ = static_cast<size_t>(vp.h + TILE_SIZE - 1) / TILE_SIZE;
	std::vector<lockfree_queue<uint32_t> > tiles(num_tiles_x_ * num_tiles_y_);

	uint32_t prim_size = 0;
	switch(primtopo)
	{
	case primitive_line_list:
	case primitive_line_strip:
		prim_size = 2;
		break;

	case primitive_triangle_list:
	case primitive_triangle_strip:
		prim_size = 3;
		break;
	}

	//变换顶点
	pparent_->get_vertex_cache()->transform_vertices(static_cast<uint32_t>(prim_count));

	atomic<int32_t> working_package(0);
#ifdef SOFTART_MULTITHEADING_ENABLED
	size_t num_threads = num_cpu_cores();

	for (size_t i = 0; i < num_threads; ++ i)
	{
		global_thread_pool().schedule(boost::bind(&geometry_assembler::dispatch_primitive_func, this, boost::ref(tiles), static_cast<int32_t>(prim_count), prim_size, boost::ref(working_package), DISPATCH_PRIMITIVE_PACKAGE_SIZE));
	}
	global_thread_pool().wait();
#else
	dispatch_primitive_func(boost::ref(tiles), static_cast<int32_t>(prim_count), prim_size, boost::ref(working_package), DISPATCH_PRIMITIVE_PACKAGE_SIZE);
#endif

	working_package = 0;
	h_pixel_shader hps = pparent_->get_pixel_shader();
#ifdef SOFTART_MULTITHEADING_ENABLED
	std::vector<h_pixel_shader> ppps(num_threads);
	for (size_t i = 0; i < num_threads; ++ i)
	{
		// create pixel_shader clone per thread from hps
		ppps[i] = hps->create_clone();
		global_thread_pool().schedule(boost::bind(&geometry_assembler::rasterize_primitive_func, this, boost::ref(tiles), ppps[i], boost::ref(working_package), RASTERIZE_PRIMITIVE_PACKAGE_SIZE));
	}
	global_thread_pool().wait();
	// destroy all pixel_shader clone
	for (size_t i = 0; i < num_threads; ++ i)
	{
		hps->destroy_clone(ppps[i]);
	}
#else
	rasterize_primitive_func(boost::ref(tiles), hps, boost::ref(working_package), RASTERIZE_PRIMITIVE_PACKAGE_SIZE);
#endif
}

END_NS_SOFTART()
