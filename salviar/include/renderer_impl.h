#ifndef SALVIAR_RENDERER_IMPL_H
#define SALVIAR_RENDERER_IMPL_H

#include <salviar/include/renderer.h>
#include <salviar/include/salviar_forward.h>

BEGIN_NS_SALVIAR();

class vertex_shader_unit;

struct state_block{
	viewport vp;	
};

class renderer_impl : public renderer
{
	//some states
	viewport vp_;
	cull_mode cm_;

	h_buffer_manager			hbufmgr_;
	h_texture_manager			htexmgr_;
	h_vertex_shader				hvs_;
	h_clipper					hclipper_;
	h_rasterizer				hrast_;
	h_pixel_shader				hps_;
	h_framebuffer				hfb_;
	h_device					hdev_;
	h_vertex_cache				hvertcache_;
	h_blend_shader				hbs_;

	h_buffer indexbuf_;
	format index_fmt_;

	primitive_topology primtopo_;

	h_rasterizer_state			hrs_;
	h_depth_stencil_state		hdss_;
	int32_t						stencil_ref_;

	vs_input_op*				vs_input_ops_;
	vs_output_op*				vs_output_ops_;

	boost::shared_ptr<shader_code> vscode_;
	boost::shared_ptr<vertex_shader_unit> vs_proto_;

	void initialize();

public:
	//inherited
	virtual h_input_layout create_input_layout(
		input_element_desc const* elem_descs, size_t elems_count,
		h_shader_code const& vs );
	
	virtual h_input_layout create_input_layout(
		input_element_desc const* elem_descs, size_t elems_count,
		h_vertex_shader const& vs );

	virtual result set_input_layout(const h_input_layout& layout);

	virtual result set_vertex_buffers(
		size_t starts_slot,
		size_t buffers_count, h_buffer const* buffers,
		size_t* strides, size_t* offsets
		);

	virtual result set_index_buffer(h_buffer hbuf, format index_fmt);
	virtual h_buffer get_index_buffer() const;
	virtual format get_index_format() const;

	virtual result set_primitive_topology(primitive_topology primtopo);
	virtual primitive_topology get_primitive_topology() const;

	virtual result set_vertex_shader(h_vertex_shader hvs);
	virtual h_vertex_shader get_vertex_shader() const;
	
	virtual result set_vertex_shader_code( boost::shared_ptr<shader_code> const& );
	virtual boost::shared_ptr<shader_code> get_vertex_shader_code() const;
	virtual result set_vs_variable( std::string const& name, void* data );
	virtual boost::shared_ptr<vertex_shader_unit> vs_proto() const;

	virtual const vs_input_op* get_vs_input_ops() const;
	virtual const vs_output_op* get_vs_output_ops() const;

	virtual result set_rasterizer_state(const h_rasterizer_state& rs);
	virtual const h_rasterizer_state& get_rasterizer_state() const;
	virtual result set_depth_stencil_state(const h_depth_stencil_state& dss, int32_t stencil_ref);
	virtual const h_depth_stencil_state& get_depth_stencil_state() const;
	virtual int32_t get_stencil_ref() const;

	virtual result set_pixel_shader(h_pixel_shader hps);
	virtual h_pixel_shader get_pixel_shader() const;

	virtual result set_blend_shader(h_blend_shader hbs);
	virtual h_blend_shader get_blend_shader();

	virtual result set_viewport(const viewport& vp);
	virtual const viewport& get_viewport() const;

	virtual result set_framebuffer_size(size_t width, size_t height, size_t num_samples);
	virtual eflib::rect<size_t> get_framebuffer_size() const;

	virtual result set_framebuffer_format(pixel_format pxfmt);
	virtual pixel_format get_framebuffer_format(pixel_format pxfmt) const;

	virtual result set_render_target_available(render_target tar, size_t tar_id, bool valid);
	virtual bool get_render_target_available(render_target tar, size_t tar_id) const;

	virtual h_framebuffer get_framebuffer() const;

	//do not support get function for a while
	virtual result set_render_target(render_target tar, size_t tar_id, surface* psurf);

	virtual h_renderer_mementor create_mementor();
	virtual result release_mementor(h_renderer_mementor& mementor);

	//these functions may used by extensions
	virtual result set_additional_state(const boost::any& state);
	virtual boost::any get_additional_state(const boost::any& name);

	virtual h_buffer create_buffer(size_t size);
	virtual result release_buffer(h_buffer& hbuf);

	virtual h_texture create_tex2d(size_t width, size_t height, size_t num_samples, pixel_format fmt);
	virtual h_texture create_texcube(size_t width, size_t height, size_t num_samples, pixel_format fmt);
	virtual result release_texture(h_texture& htex);

	virtual h_sampler create_sampler(const sampler_desc& desc);
	virtual result release_sampler(h_sampler& hsmp);

	virtual result draw(size_t startpos, size_t primcnt);
	virtual result draw_index(size_t startpos, size_t primcnt, int basevert);

	virtual result clear_color(size_t tar_id, const color_rgba32f& c);
	virtual result clear_depth(float d);
	virtual result clear_stencil(uint32_t s);
	virtual result clear_color(size_t tar_id, const eflib::rect<size_t>& rc, const color_rgba32f& c);
	virtual result clear_depth(const eflib::rect<size_t>& rc, float d);
	virtual result clear_stencil(const eflib::rect<size_t>& rc, uint32_t s);

	virtual result present();

	//this class for inner system
	renderer_impl(const renderer_parameters* pparam, h_device hdev);

	h_buffer_manager get_buf_mgr();
	h_texture_manager get_tex_mgr();

	h_rasterizer get_rasterizer();
	
	h_device get_device();
	h_vertex_cache get_vertex_cache();
	h_clipper get_clipper();
};

END_NS_SALVIAR()

#endif