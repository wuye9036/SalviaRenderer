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

#include <vector>
#include "softart_fwd.h"
BEGIN_NS_SOFTART()


class geometry_assembler : public render_stage
{
	void dispatch_primitive_func(std::vector<lockfree_queue<uint32_t> >& tiles, int32_t prim_count, uint32_t stride, atomic<int32_t>& working_package, int32_t package_size);
	void rasterize_primitive_func(std::vector<lockfree_queue<uint32_t> >& tiles, const h_pixel_shader& pps, atomic<int32_t>& working_package, int32_t package_size);

	int num_tiles_x_, num_tiles_y_;

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
