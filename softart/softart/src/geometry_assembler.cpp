#include "../include/geometry_assembler.h"

#include "eflib/include/eflib.h"

#include "../include/shaderregs_op.h"
#include "../include/shader.h"
#include "../include/renderer_impl.h"

#include "../include/stream.h"
#include "../include/buffer.h"
#include "../include/clipper.h"
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
const int GEOMETRY_SETUP_PACKAGE_SIZE = 1;
const int DISPATCH_PRIMITIVE_PACKAGE_SIZE = 1;
const int RASTERIZE_PRIMITIVE_PACKAGE_SIZE = 1;

void geometry_assembler::initialize(renderer_impl* pparent)
{
	custom_assert(pparent, "初始化出现异常，传入的初始化指针为空！");

	pparent_ = pparent;
}

geometry_assembler::geometry_assembler()
{}

void geometry_assembler::geometry_setup_func(std::vector<uint32_t>& num_clipped_prims, std::vector<vs_output>& clipped_verts, int32_t prim_count, uint32_t stride,
		atomic<int32_t>& working_package, int32_t package_size){

	const int32_t num_packages = (prim_count + package_size - 1) / package_size;

	const cull_mode cm = pparent_->get_rasterizer()->get_cull_mode();
	const h_vertex_cache& dvc = pparent_->get_vertex_cache();
	h_clipper clipper = pparent_->get_clipper();

	int32_t local_working_package = working_package ++;
	while (local_working_package < num_packages){
		const int32_t start = local_working_package * package_size;
		const int32_t end = min(prim_count, start + package_size);
		for (int32_t i = start; i < end; ++ i){
			if (3 == stride){
				vs_output pv[3];
				vec2 pv_2d[3];
				for (size_t j = 0; j < 3; ++ j){
					const vs_output& v = dvc->fetch(i * 3 + j);
					pv[j] = v;
					const float sign_w = sign(v.wpos.w);
					const float x = v.wpos.x * sign_w;
					const float y = v.wpos.y * sign_w;
					pv_2d[j] = vec2(x, y);
				}

				bool culled = false;
				const float area = cross_prod2(pv_2d[1] - pv_2d[0], pv_2d[2] - pv_2d[0]);
				if (cm != cull_none){
					if (((cm == cull_front) && (area >= 0))
							|| ((cm == cull_back) && (area <= 0))){
						culled = true;
					}
				}

				if (!culled){
					vector<vs_output> tmp_verts;
					clipper->clip(tmp_verts, pparent_->get_viewport(), pv[0], pv[1], pv[2]);
					custom_assert(tmp_verts.size() < 21, "");

					num_clipped_prims[i] = static_cast<uint32_t>(tmp_verts.size() - 2);

					bool flip = (area < 0);
					for(int i_tri = 1; i_tri < static_cast<int>(tmp_verts.size()) - 1; ++ i_tri){
						clipped_verts[i * 21 + (i_tri - 1) * 3 + 0] = tmp_verts[0];
						if (flip){
							clipped_verts[i * 21 + (i_tri - 1) * 3 + 1] = tmp_verts[i_tri + 1];
							clipped_verts[i * 21 + (i_tri - 1) * 3 + 2] = tmp_verts[i_tri + 0];
						}
						else{
							clipped_verts[i * 21 + (i_tri - 1) * 3 + 1] = tmp_verts[i_tri + 0];
							clipped_verts[i * 21 + (i_tri - 1) * 3 + 2] = tmp_verts[i_tri + 1];
						}
					}
				}
				else{
					num_clipped_prims[i] = 0;
				}
			}
			else{
				num_clipped_prims[i] = 1;
				for (size_t j = 0; j < stride; ++ j){
					clipped_verts[i * 21 + j] = dvc->fetch(i * stride + j);
				}
			}
		}

		local_working_package = working_package ++;
	}
}

void geometry_assembler::dispatch_primitive_func(std::vector<lockfree_queue<uint32_t> >& tiles, int num_tiles_x, int num_tiles_y,
		const std::vector<vs_output>& clipped_verts, int32_t prim_count, uint32_t stride, atomic<int32_t>& working_package, int32_t package_size){

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
			vec2 pv[3];
			for (size_t j = 0; j < stride; ++ j){
				pv[j] = clipped_verts[i * stride + j].wpos.xy();
			}

			x_min = pv[0].x;
			x_max = pv[0].x;
			y_min = pv[0].y;
			y_max = pv[0].y;
			for (size_t j = 1; j < stride; ++ j){
				x_min = min(x_min, pv[j].x);
				x_max = max(x_max, pv[j].x);
				y_min = min(y_min, pv[j].y);
				y_max = max(y_max, pv[j].y);
			}

			const int sx = std::min(fast_floori(std::max(0.0f, x_min) / TILE_SIZE), num_tiles_x);
			const int sy = std::min(fast_floori(std::max(0.0f, y_min) / TILE_SIZE), num_tiles_y);
			const int ex = std::min(fast_ceili(std::max(0.0f, x_max) / TILE_SIZE) + 1, num_tiles_x);
			const int ey = std::min(fast_ceili(std::max(0.0f, y_max) / TILE_SIZE) + 1, num_tiles_y);
			if ((sx + 1 == ex) && (sy + 1 == ey)){
				// Small primitive
				tiles[sy * num_tiles_x + sx].enqueue(i);
			}
			else{
				if (3 == stride){
					// x * (y1 - y0) - y * (x1 - x0) - (y1 * x0 - x1 * y0)
					vec3 edge_factors[3];
					for (int e = 0; e < 3; ++ e){
						const int se = e;
						const int ee = (e + 1) % 3;
						edge_factors[e].x = pv[ee].y - pv[se].y;
						edge_factors[e].y = pv[ee].x - pv[se].x;
						edge_factors[e].z = pv[ee].y * pv[se].x - pv[ee].x * pv[se].y;
					}

					for (int y = sy; y < ey; ++ y){
						for (int x = sx; x < ex; ++ x){
							int2 corners[] = {
								int2((x + 0) * TILE_SIZE, (y + 0) * TILE_SIZE),
								int2((x + 1) * TILE_SIZE, (y + 0) * TILE_SIZE),
								int2((x + 0) * TILE_SIZE, (y + 1) * TILE_SIZE),
								int2((x + 1) * TILE_SIZE, (y + 1) * TILE_SIZE)
							};

							bool intersect = true;
							// Trival rejection
							for (int e = 0; intersect && (e < 3); ++ e){
								int min_c = (edge_factors[e].y > 0) * 2 + (edge_factors[e].x <= 0);
								if (corners[min_c].x * edge_factors[e].x
										- corners[min_c].y * edge_factors[e].y
										- edge_factors[e].z > 0){
									intersect = false;
								}
							}

							if (intersect){
								tiles[y * num_tiles_x + x].enqueue(i);
							}
						}
					}
				}
				else{
					for (int y = sy; y < ey; ++ y){
						for (int x = sx; x < ex; ++ x){
							tiles[y * num_tiles_x + x].enqueue(i);
						}
					}
				}
			}
		}

		local_working_package = working_package ++;
	}
}

void geometry_assembler::rasterize_primitive_func(std::vector<lockfree_queue<uint32_t> >& tiles, int num_tiles_x,
		const std::vector<vs_output>& clipped_verts, const h_pixel_shader& pps, atomic<int32_t>& working_package, int32_t package_size)
{
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
		for (int32_t i = start; i < end; ++ i){
			lockfree_queue<uint32_t>& prims = tiles[i];

			std::vector<uint32_t> sorted_prims;
			prims.dequeue_all(std::back_insert_iterator<std::vector<uint32_t> >(sorted_prims));
			std::sort(sorted_prims.begin(), sorted_prims.end());

			int y = i / num_tiles_x;
			int x = i - y * num_tiles_x;
			tile_vp.x = static_cast<float>(x * TILE_SIZE);
			tile_vp.y = static_cast<float>(y * TILE_SIZE);

			rasterize_func_(this, clipped_verts, hrast, sorted_prims, tile_vp, pps);
		}

		local_working_package = working_package ++;
	}
}

void geometry_assembler::rasterize_line_func(const std::vector<vs_output>& clipped_verts, const h_rasterizer& hrast, const std::vector<uint32_t>& sorted_prims, const viewport& tile_vp, const h_pixel_shader& pps){
	for (std::vector<uint32_t>::const_iterator iter = sorted_prims.begin(); iter != sorted_prims.end(); ++ iter){
		uint32_t iprim = *iter;
		hrast->rasterize_line(clipped_verts[iprim * 2 + 0], clipped_verts[iprim * 2 + 1], tile_vp, pps);
	}
}

void geometry_assembler::rasterize_triangle_func(const std::vector<vs_output>& clipped_verts, const h_rasterizer& hrast, const std::vector<uint32_t>& sorted_prims, const viewport& tile_vp, const h_pixel_shader& pps){
	for (std::vector<uint32_t>::const_iterator iter = sorted_prims.begin(); iter != sorted_prims.end(); ++ iter){
		uint32_t iprim = *iter;
		hrast->rasterize_triangle(clipped_verts[iprim * 3 + 0], clipped_verts[iprim * 3 + 1], clipped_verts[iprim * 3 + 2], tile_vp, pps);
	}
}

void geometry_assembler::draw(size_t prim_count){

	custom_assert(pparent_, "Renderer 指针为空！可能该对象没有经过正确的初始化！");
	if(!pparent_) return;

	const h_rasterizer& hrast = pparent_->get_rasterizer();
	if(!hrast) return;

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
	
	uint32_t prim_size = 0;
	switch(primtopo)
	{
	case primitive_line_list:
	case primitive_line_strip:
		prim_size = 2;
		rasterize_func_ = boost::mem_fn(&geometry_assembler::rasterize_line_func);
		break;
	case primitive_triangle_list:
	case primitive_triangle_strip:
		prim_size = 3;
		rasterize_func_ = boost::mem_fn(&geometry_assembler::rasterize_triangle_func);
		break;
	default:
		custom_assert(false, "枚举值无效：无效的Primitive Topology");
		return;
	}

	const viewport& vp = pparent_->get_viewport();
	int num_tiles_x = static_cast<size_t>(vp.w + TILE_SIZE - 1) / TILE_SIZE;
	int num_tiles_y = static_cast<size_t>(vp.h + TILE_SIZE - 1) / TILE_SIZE;
	std::vector<lockfree_queue<uint32_t> > tiles(num_tiles_x * num_tiles_y);

	// Vertex transform
	pparent_->get_vertex_cache()->transform_vertices(static_cast<uint32_t>(prim_count));

	atomic<int32_t> working_package(0);
	size_t num_threads = num_available_threads();

	// Culling, Clipping, Geometry setup
	std::vector<uint32_t> num_clipped_prims(prim_count);
	std::vector<vs_output> clipped_verts_full(prim_count * 21);
	for (size_t i = 0; i < num_threads - 1; ++ i){
		global_thread_pool().schedule(boost::bind(&geometry_assembler::geometry_setup_func, this, boost::ref(num_clipped_prims),
			boost::ref(clipped_verts_full), static_cast<int32_t>(prim_count), prim_size,
			working_package, GEOMETRY_SETUP_PACKAGE_SIZE));
	}
	geometry_setup_func(boost::ref(num_clipped_prims), boost::ref(clipped_verts_full), static_cast<int32_t>(prim_count),
		prim_size, working_package, GEOMETRY_SETUP_PACKAGE_SIZE);
	global_thread_pool().wait();

	std::vector<vs_output> clipped_verts;
	for (size_t i = 0; i < prim_count; ++ i){
		for (uint32_t j = 0; j < num_clipped_prims[i]; ++ j){
			for (uint32_t k = 0; k < prim_size; ++ k){
				clipped_verts.push_back(clipped_verts_full[i * 21 + j * prim_size + k]);
			}
		}
	}

	working_package = 0;
	for (size_t i = 0; i < num_threads - 1; ++ i){
		global_thread_pool().schedule(boost::bind(&geometry_assembler::dispatch_primitive_func, this, boost::ref(tiles),
			num_tiles_x, num_tiles_y, boost::ref(clipped_verts), static_cast<int32_t>(clipped_verts.size() / prim_size),
			prim_size, boost::ref(working_package), DISPATCH_PRIMITIVE_PACKAGE_SIZE));
	}
	dispatch_primitive_func(boost::ref(tiles), num_tiles_x, num_tiles_y,
		boost::ref(clipped_verts), static_cast<int32_t>(clipped_verts.size() / prim_size),
		prim_size, boost::ref(working_package), DISPATCH_PRIMITIVE_PACKAGE_SIZE);
	global_thread_pool().wait();

	working_package = 0;
	h_pixel_shader hps = pparent_->get_pixel_shader();
	std::vector<h_pixel_shader> ppps(num_threads - 1);
	for (size_t i = 0; i < num_threads - 1; ++ i){
		// create pixel_shader clone per thread from hps
		ppps[i] = hps->create_clone();
		global_thread_pool().schedule(boost::bind(&geometry_assembler::rasterize_primitive_func, this, boost::ref(tiles),
			num_tiles_x, boost::ref(clipped_verts), ppps[i], boost::ref(working_package), RASTERIZE_PRIMITIVE_PACKAGE_SIZE));
	}
	rasterize_primitive_func(boost::ref(tiles), num_tiles_x, boost::ref(clipped_verts), hps, boost::ref(working_package), RASTERIZE_PRIMITIVE_PACKAGE_SIZE);
	global_thread_pool().wait();
	// destroy all pixel_shader clone
	for (size_t i = 0; i < num_threads - 1; ++ i){
		hps->destroy_clone(ppps[i]);
	}
}

END_NS_SOFTART()
