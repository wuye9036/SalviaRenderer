#ifndef SOFTART_RASTERIZER_H
#define SOFTART_RASTERIZER_H

#include "decl.h"
#include "enums.h"
#include "shader.h"
#include "framebuffer.h"

#include "boost/smart_ptr.hpp"

struct scanline_info;

class rasterizer : public render_stage
{
	cull_mode cm_;
	fill_mode fm_;

	h_pixel_shader hps_;
	h_framebuffer hfb_;

	//线光栅化。光栅化后的点将直接传到PS中处理。
	void rasterize_line_impl(const vs_output& v0, const vs_output& v1);
	void rasterize_triangle_impl(const vs_output& v0, const vs_output& v1, const vs_output& v2);
	void rasterize_scanline_impl(const scanline_info& sl);

public:
	//inherited
	void initialize(renderer_impl* pparent);

	PROC_RS_UPDATED(pixel_shader)

	//constructor
	rasterizer();

	//state_seter
	void set_cull_mode(cull_mode cm);
	void set_fill_mode(fill_mode fm);

	//drawer
	void rasterize_line(const vs_output& v0, const vs_output& v1);
	void rasterize_triangle(const vs_output& v0, const vs_output& v1, const vs_output& v2);
};

//DECL_HANDLE(rasterizer, h_rasterizer);

#endif