#ifndef SOFTART_RENDERER_H
#define SOFTART_RENDERER_H

#include "../include/decl.h"
#include "../include/enums.h"
#include "../include/colors.h"

#include "eflib/include/eflib.h"

#ifdef EFLIB_MSVC
#pragma warning(push)
#pragma warning(disable : 6011)
#endif
#include <boost/smart_ptr.hpp>
#ifdef EFLIB_MSVC
#pragma warning(pop)
#endif
#include <boost/any.hpp>

#include <vector>
#include "softart_fwd.h"
BEGIN_NS_SOFTART()


struct viewport{
	float x;
	float y;
	float w;
	float h;
	float minz;
	float maxz;
};

struct renderer_parameters
{
	size_t backbuffer_width;
	size_t backbuffer_height;

	pixel_format backbuffer_format;
};

typedef std::vector<input_element_decl> input_layout_decl;
enum stream_index;

class renderer
{
public:
	//state accessors
	/////////////////////////////////////////////
	virtual result set_input_layout(const input_layout_decl& layout) = 0;
	virtual const input_layout_decl& get_input_layout() const = 0;

	virtual result set_stream(stream_index sidx, h_buffer hbuf) = 0;
	virtual h_buffer get_stream(stream_index sidx) const = 0;

	virtual result set_index_buffer(h_buffer hbuf, index_type idxtype) = 0;
	virtual h_buffer get_index_buffer() const = 0;
	virtual index_type get_index_type() const = 0;

	virtual result set_primitive_topology(primitive_topology primtopo) = 0;
	virtual primitive_topology get_primitive_topology() const = 0;

	virtual result set_vertex_shader(h_vertex_shader hvs) = 0;
	virtual h_vertex_shader get_vertex_shader() const = 0;

	virtual result set_rasterizer_state(const h_rasterizer_state& rs) = 0;
	virtual const h_rasterizer_state& get_rasterizer_state() const = 0;

	virtual result set_pixel_shader(h_pixel_shader hps) = 0;
	virtual h_pixel_shader get_pixel_shader() const = 0;

	virtual result set_blend_shader(h_blend_shader hbs) = 0;
	virtual h_blend_shader get_blend_shader() = 0;

	virtual result set_viewport(const viewport& vp) = 0;
	virtual const viewport& get_viewport() const = 0;

	virtual result set_framebuffer_size(size_t width, size_t height) = 0;
	virtual efl::rect<size_t> get_framebuffer_size() const = 0;

	virtual result set_framebuffer_format(pixel_format pxfmt) = 0;
	virtual pixel_format get_framebuffer_format(pixel_format pxfmt) const = 0;

	virtual result set_render_target_available(render_target tar, size_t tar_id, bool valid) = 0;
	virtual bool get_render_target_available(render_target tar, size_t tar_id) const = 0;

	virtual h_framebuffer get_framebuffer() const = 0;

	//do not support get function for a while
	virtual result set_render_target(render_target tar, size_t tar_id, surface* psurf) = 0;

	virtual h_renderer_mementor create_mementor() = 0;
	virtual result release_mementor(h_renderer_mementor& mementor) = 0;

	//these functions may be used by extensions
	virtual result set_additional_state(const boost::any& state) = 0;
	virtual boost::any get_additional_state(const boost::any& name) = 0;

	//resource managements
	//////////////////////////////////////////////
	virtual h_buffer create_buffer(size_t size) = 0;
	virtual result release_buffer(h_buffer& hbuf) = 0;

	virtual h_texture create_tex2d(size_t width, size_t height, pixel_format fmt) = 0;
	virtual h_texture create_texcube(size_t width, size_t height, pixel_format fmt) = 0;
	virtual result release_texture(h_texture& htex) = 0;

	virtual h_sampler create_sampler(const sampler_desc& desc) = 0;
	virtual result release_sampler(h_sampler& hsmp) = 0;

	//render operations
	//////////////////////////////////////////////
	virtual result draw(size_t startpos, size_t primcnt) = 0;
	virtual result draw_index(size_t startpos, size_t primcnt, int basevert) = 0;

	virtual result clear_color(size_t tar_id, const color_rgba32f& c) = 0;
	virtual result clear_depth(float d) = 0;
	virtual result clear_stencil(uint32_t s) = 0;
	virtual result clear_color(size_t tar_id, const efl::rect<size_t>& rc, const color_rgba32f& c) = 0;
	virtual result clear_depth(const efl::rect<size_t>& rc, float d) = 0;
	virtual result clear_stencil(const efl::rect<size_t>& rc, uint32_t s) = 0;

	virtual result present() = 0;
};



h_renderer create_software_renderer(const renderer_parameters* pparam, h_device hdev);

END_NS_SOFTART()

#endif