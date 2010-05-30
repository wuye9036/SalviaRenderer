#ifndef SOFTART_RASTERIZER_H
#define SOFTART_RASTERIZER_H

#include "decl.h"
#include "enums.h"
#include "shader.h"
#include "framebuffer.h"

#include <boost/smart_ptr.hpp>
#include <boost/function.hpp>
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

	boost::function<bool (float)> cm_func_;
	boost::function<void (uint32_t&, vs_output*, const h_clipper&, const vs_output*, float)> clipping_func_;

public:
	rasterizer_state(const rasterizer_desc& desc);
	const rasterizer_desc& get_desc() const;

	bool cull(float area) const;
	void clipping(uint32_t& num_clipped_prims, vs_output* clipped_verts, const h_clipper& clipper, const vs_output* pv, float area);
};

class rasterizer : public render_stage
{
	h_rasterizer_state state_;

	h_framebuffer hfb_;

	//线光栅化。光栅化后的点将直接传到PS中处理。
	void rasterize_scanline_impl(const scanline_info& sl, const h_pixel_shader& pps);


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
};

//DECL_HANDLE(rasterizer, h_rasterizer);

END_NS_SOFTART()

#endif