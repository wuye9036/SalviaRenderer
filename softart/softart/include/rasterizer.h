#ifndef SOFTART_RASTERIZER_H
#define SOFTART_RASTERIZER_H

#include "decl.h"
#include "enums.h"
#include "shader.h"
#include "framebuffer.h"
#include "atomic.h"
#include "lockfree_queue.h"

#include <boost/array.hpp>
#include <boost/function.hpp>
#ifdef EFLIB_MSVC
#pragma warning(push)
#pragma warning(disable : 6011)
#endif
#include <boost/smart_ptr.hpp>
#ifdef EFLIB_MSVC
#pragma warning(pop)
#endif

#include "softart_fwd.h"
BEGIN_NS_SOFTART()


struct scanline_info;

struct rasterizer_desc {
	fill_mode fm;
	cull_mode cm;
	bool front_ccw;
	int32_t depth_bias;
	float depth_bias_clamp;
	float slope_scaled_depth_bias;
	bool depth_clip_enable;
	bool scissor_enable;
	bool multisample_enable;
	bool anti_aliased_line_enable;

	rasterizer_desc()
		: fm(fill_solid), cm(cull_back),
			front_ccw(false),
			depth_bias(0), depth_bias_clamp(0), slope_scaled_depth_bias(0),
			depth_clip_enable(true), scissor_enable(false),
			multisample_enable(true), anti_aliased_line_enable(false)
	{
	}
};

class rasterizer_state {
	rasterizer_desc desc_;

	typedef bool (*cm_func_type)(float area);
	typedef void (*clipping_func_type)(uint32_t& num_clipped_prims, vs_output* clipped_verts, const h_clipper& clipper, const vs_output* pv, float area);
	typedef void (*triangle_rast_func_type)(uint32_t&, boost::function<void (rasterizer*, const std::vector<vs_output>&, const std::vector<uint32_t>&, const viewport&, const h_pixel_shader&)>&);

	cm_func_type cm_func_;
	clipping_func_type clipping_func_;
	triangle_rast_func_type triangle_rast_func_;

public:
	rasterizer_state(const rasterizer_desc& desc);
	const rasterizer_desc& get_desc() const;

	bool cull(float area) const;
	void clipping(uint32_t& num_clipped_prims, vs_output* clipped_verts, const h_clipper& clipper, const vs_output* pv, float area) const;
	void triangle_rast_func(uint32_t& prim_size, boost::function<void (rasterizer*, const std::vector<vs_output>&, const std::vector<uint32_t>&, const viewport&, const h_pixel_shader&)>& rasterize_func) const;
};

class rasterizer : public render_stage
{
	h_rasterizer_state state_;

	h_framebuffer hfb_;

	//线光栅化。光栅化后的点将直接传到PS中处理。
	void rasterize_scanline_impl(const scanline_info& sl, const h_pixel_shader& pps);

	void geometry_setup_func(std::vector<uint32_t>& num_clipped_prims, std::vector<vs_output>& clipped_verts, int32_t prim_count, primitive_topology primtopo,
		atomic<int32_t>& working_package, int32_t package_size);
	void dispatch_primitive_func(std::vector<lockfree_queue<uint32_t> >& tiles, int num_tiles_x, int num_tiles_y,
		const std::vector<vs_output>& clipped_verts, int32_t prim_count, uint32_t stride, atomic<int32_t>& working_package, int32_t package_size);
	void rasterize_primitive_func(std::vector<lockfree_queue<uint32_t> >& tiles, int num_tiles_x, const std::vector<vs_output>& clipped_verts,
		const h_pixel_shader& pps, atomic<int32_t>& working_package, int32_t package_size);

	boost::function<void (rasterizer*, const std::vector<vs_output>&, const std::vector<uint32_t>&, const viewport&, const h_pixel_shader&)> rasterize_func_;

public:
	//inherited
	void initialize(renderer_impl* pparent);

	//constructor
	rasterizer();
	~rasterizer();

	//state_seter
	void set_state(const h_rasterizer_state& state);
	const h_rasterizer_state& get_state() const;

	//drawer
	void rasterize_line(const vs_output& v0, const vs_output& v1, const viewport& vp, const h_pixel_shader& pps);
	void rasterize_triangle(const vs_output& v0, const vs_output& v1, const vs_output& v2, const viewport& vp, const h_pixel_shader& pps);

	void rasterize_line_func(const std::vector<vs_output>& clipped_verts, const std::vector<uint32_t>& sorted_prims, const viewport& tile_vp, const h_pixel_shader& pps);
	void rasterize_triangle_func(const std::vector<vs_output>& clipped_verts, const std::vector<uint32_t>& sorted_prims, const viewport& tile_vp, const h_pixel_shader& pps);

	//绘制函数
	void draw(size_t prim_count);
};

//DECL_HANDLE(rasterizer, h_rasterizer);

END_NS_SOFTART()

#endif