#ifndef SOFTART_RENDERER_IMPL_H
#define SOFTART_RENDERER_IMPL_H

#include "renderer.h"

struct state_block
{
	viewport vp;
	
};

class renderer_impl : public renderer
{
	//some states
	viewport vp_;
	cull_mode cm_;

	h_buffer_manager			hbufmgr_;
	h_texture_manager			htexmgr_;
	h_geometry_assembler		hga_;
	h_vertex_shader				hvs_;
	h_clipper						hclipper_;
	h_rasterizer						hrast_;
	h_pixel_shader				hps_;
	h_framebuffer					hfb_;
	h_device						hdev_;
	h_vertex_cache				hvertcache_;
	h_blend_shader				hbs_;

	void initialize();

public:
	//inherited
	virtual result set_input_layout(const input_layout_decl& layout);
	virtual const input_layout_decl& get_input_layout() const;

	virtual result set_stream(stream_index sidx, h_buffer hbuf);
	virtual h_buffer get_stream(stream_index sidx) const;

	virtual result set_index_buffer(h_buffer hbuf, index_type idxtype);
	virtual h_buffer get_index_buffer() const;
	virtual index_type get_index_type() const;

	virtual result set_primitive_topology(primitive_topology primtopo);
	virtual primitive_topology get_primitive_topology() const;

	virtual result set_vertex_shader(h_vertex_shader hvs);
	virtual h_vertex_shader get_vertex_shader() const;

	virtual result set_cull_mode(cull_mode cm);
	virtual cull_mode get_cull_mode() const;

	virtual result set_fill_mode(fill_mode fm);
	virtual fill_mode get_fill_mode() const;

	virtual result set_pixel_shader(h_pixel_shader hps);
	virtual h_pixel_shader get_pixel_shader() const;

	virtual result set_blend_shader(h_blend_shader hbs);
	virtual h_blend_shader get_blend_shader();

	virtual result set_viewport(const viewport& vp);
	virtual const viewport& get_viewport() const;

	virtual result set_framebuffer_size(size_t width, size_t height);
	virtual efl::rect<size_t> get_framebuffer_size() const;

	virtual result set_framebuffer_format(pixel_format pxfmt);
	virtual pixel_format get_framebuffer_format(pixel_format pxfmt) const;

	virtual result set_render_target_available(render_target tar, size_t tar_id, bool valid);
	virtual bool get_render_target_available(render_target tar, size_t tar_id) const;

	//do not support get function for a while
	virtual result set_render_target(render_target tar, size_t tar_id, surface* psurf);

	virtual h_renderer_mementor create_mementor();
	virtual result release_mementor(h_renderer_mementor& mementor);

	//these functions may used by extensions
	virtual result set_additional_state(const boost::any& state);
	virtual boost::any get_additional_state(const boost::any& name);

	virtual h_buffer create_buffer(size_t size);
	virtual result release_buffer(h_buffer& hbuf);

	virtual h_texture create_tex2d(size_t width, size_t height, pixel_format fmt);
	virtual h_texture create_texcube(size_t width, size_t height, pixel_format fmt);
	virtual result release_texture(h_texture& htex);

	virtual h_sampler create_sampler();
	virtual result release_sampler(h_sampler& hsmp);

	virtual result draw(size_t startpos, size_t primcnt);
	virtual result draw_index(size_t startpos, size_t primcnt, int basevert);

	virtual result clear_color(size_t tar_id, const color_rgba32f& c);
	virtual result clear_depth(float d);
	virtual result clear_stencil(uint32_t s);
	virtual result clear_color(size_t tar_id, const efl::rect<size_t>& rc, const color_rgba32f& c);
	virtual result clear_depth(const efl::rect<size_t>& rc, float d);
	virtual result clear_stencil(const efl::rect<size_t>& rc, uint32_t s);

	virtual result present();

	//this class for inner system
	renderer_impl(const renderer_parameters* pparam, h_device hdev);

	h_buffer_manager get_buf_mgr();
	h_texture_manager get_tex_mgr();

	h_geometry_assembler get_geometry_assembler();
	h_rasterizer get_rasterizer();
	h_framebuffer get_framebuffer();
	h_device get_device();
	h_vertex_cache get_vertex_cache();
	h_clipper get_clipper();
};

#endif