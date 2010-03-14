#ifndef SOFTART_RASTERIZER_H
#define SOFTART_RASTERIZER_H

#include "decl.h"
#include "enums.h"
#include "shader.h"
#include "framebuffer.h"

#include <boost/smart_ptr.hpp>
#include <boost/thread.hpp>
#include "softart_fwd.h"
BEGIN_NS_SOFTART()


struct scanline_info;

class rasterizer : public render_stage
{
	cull_mode cm_;
	fill_mode fm_;

	h_pixel_shader hps_;
	h_framebuffer hfb_;

	// one ps per thread , ps create by create_ps_per_thread hps_->create_clone()
	pixel_shader **pps_clones_;
	size_t num_ps_clones_;

	//线光栅化。光栅化后的点将直接传到PS中处理。
	void rasterize_line_impl(const vs_output& v0, const vs_output& v1, const viewport& vp, size_t thread_index);
	void rasterize_triangle_impl(const vs_output& v0, const vs_output& v1, const vs_output& v2, const viewport& vp, size_t thread_index);
	void rasterize_scanline_impl(const scanline_info& sl, size_t thread_index);

	pixel_shader *get_ps(size_t thread_index)
	{
		custom_assert(thread_index == 0 || (thread_index > 0 && thread_index - 1 < num_ps_clones_), "wrong thread_index");
		return (thread_index == 0 ? hps_.get() : pps_clones_[thread_index - 1]);
	}

public:
	//inherited
	void initialize(renderer_impl* pparent);

	PROC_RS_UPDATED(pixel_shader)

	//constructor
	rasterizer();
	~rasterizer();

	//state_seter
	void set_cull_mode(cull_mode cm);
	void set_fill_mode(fill_mode fm);

	//drawer
	void rasterize_line(const vs_output& v0, const vs_output& v1, const viewport& vp, size_t thread_index);
	void rasterize_triangle(const vs_output& v0, const vs_output& v1, const vs_output& v2, const viewport& vp, size_t thread_index);

	void create_ps_clones(size_t num_of_clones);
	void destroy_ps_clones();

};

//DECL_HANDLE(rasterizer, h_rasterizer);

END_NS_SOFTART()

#endif