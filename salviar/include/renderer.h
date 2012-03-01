#ifndef SALVIAR_RENDERER_H
#define SALVIAR_RENDERER_H

#include <salviar/include/decl.h>
#include <salviar/include/enums.h>
#include <salviar/include/colors.h>
#include <salviar/include/format.h>

#include <eflib/include/math/collision_detection.h>

#include <eflib/include/platform/disable_warnings.h>
#include <boost/any.hpp>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/enable_warnings.h>

#include <vector>
#include <salviar/include/salviar_forward.h>

BEGIN_NS_SALVIAR();

class shader_code;

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
	size_t backbuffer_num_samples;

	pixel_format backbuffer_format;
};

class renderer
{
public:
	//state accessors
	/////////////////////////////////////////////
	virtual result set_input_layout( h_input_layout const& layout) = 0;

	virtual h_input_layout create_input_layout(
		input_element_desc const* elem_descs, size_t elems_count,
		h_shader_code const& code ) = 0;

	virtual h_input_layout create_input_layout(
		input_element_desc const* elem_descs, size_t elems_count,
		h_vertex_shader const& vs ) = 0;

	virtual result set_vertex_buffers(
		size_t starts_slot,
		size_t buffers_count, h_buffer const* buffers,
		size_t* strides, size_t* offsets ) = 0;

	virtual result set_index_buffer(h_buffer hbuf, format index_fmt) = 0;
	virtual h_buffer get_index_buffer() const = 0;
	virtual format get_index_format() const = 0;

	virtual result set_primitive_topology(primitive_topology primtopo) = 0;
	virtual primitive_topology get_primitive_topology() const = 0;

	virtual result set_vertex_shader(h_vertex_shader hvs) = 0;
	virtual h_vertex_shader get_vertex_shader() const = 0;

	virtual result set_vertex_shader_code( h_shader_code const& ) = 0;
	virtual h_shader_code get_vertex_shader_code() const = 0;
	virtual result set_vs_variable( std::string const& name, void* data ) = 0;

	virtual result set_rasterizer_state(const h_rasterizer_state& rs) = 0;
	virtual const h_rasterizer_state& get_rasterizer_state() const = 0;

	virtual result set_pixel_shader(h_pixel_shader hps) = 0;
	virtual h_pixel_shader get_pixel_shader() const = 0;

	virtual result set_pixel_shader_code( h_shader_code const& ) = 0;
	virtual h_shader_code get_pixel_shader_code() const = 0;
	virtual result set_ps_variable( std::string const& name, void* data ) = 0;

	virtual result set_blend_shader(h_blend_shader hbs) = 0;
	virtual h_blend_shader get_blend_shader() = 0;

	virtual result set_viewport(const viewport& vp) = 0;
	virtual const viewport& get_viewport() const = 0;

	virtual result set_framebuffer_size(size_t width, size_t height, size_t num_samples) = 0;
	virtual eflib::rect<size_t> get_framebuffer_size() const = 0;

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

	virtual h_texture create_tex2d(size_t width, size_t height, size_t num_samples, pixel_format fmt) = 0;
	virtual h_texture create_texcube(size_t width, size_t height, size_t num_samples, pixel_format fmt) = 0;
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
	virtual result clear_color(size_t tar_id, const eflib::rect<size_t>& rc, const color_rgba32f& c) = 0;
	virtual result clear_depth(const eflib::rect<size_t>& rc, float d) = 0;
	virtual result clear_stencil(const eflib::rect<size_t>& rc, uint32_t s) = 0;

	virtual result present() = 0;
};

h_renderer create_software_renderer(const renderer_parameters* pparam, h_device hdev);

END_NS_SALVIAR()

#endif