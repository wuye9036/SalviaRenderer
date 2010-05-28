#ifndef SOFTART_GEOMETRY_ASSEMBLER_H
#define SOFTART_GEOMETRY_ASSEMBLER_H

#include "render_stage.h"

#include "renderer_capacity.h"
#include "decl.h"
#include "enums.h"
#include "handles.h"

#include "atomic.h"
#include "lockfree_queue.h"

#include <boost/array.hpp>
#include <boost/function.hpp>

#include <vector>
#include "softart_fwd.h"
BEGIN_NS_SOFTART()


class geometry_assembler : public render_stage
{
	void geometry_setup_func(std::vector<uint32_t>& num_clipped_prims, std::vector<vs_output>& clipped_verts, int32_t prim_count, uint32_t stride,
		atomic<int32_t>& working_package, int32_t package_size);
	void dispatch_primitive_func(std::vector<lockfree_queue<uint32_t> >& tiles, int num_tiles_x, int num_tiles_y,
		const std::vector<vs_output>& clipped_verts, int32_t prim_count, uint32_t stride, atomic<int32_t>& working_package, int32_t package_size);
	void rasterize_primitive_func(std::vector<lockfree_queue<uint32_t> >& tiles, int num_tiles_x, const std::vector<vs_output>& clipped_verts,
		const h_pixel_shader& pps, atomic<int32_t>& working_package, int32_t package_size);

	void rasterize_line_func(const std::vector<vs_output>& clipped_verts, const h_rasterizer& hrast, const std::vector<uint32_t>& sorted_prims, const viewport& tile_vp, const h_pixel_shader& pps);
	void rasterize_triangle_func(const std::vector<vs_output>& clipped_verts, const h_rasterizer& hrast, const std::vector<uint32_t>& sorted_prims, const viewport& tile_vp, const h_pixel_shader& pps);

	boost::function<void (geometry_assembler*, const std::vector<vs_output>&, const h_rasterizer&, const std::vector<uint32_t>&, const viewport&, const h_pixel_shader&)> rasterize_func_;

public:
	//
	//Inherited
	void initialize(renderer_impl* pparent);

	//构造函数
	geometry_assembler();

	//绘制函数
	void draw(size_t prim_count);
};

END_NS_SOFTART()

#endif
