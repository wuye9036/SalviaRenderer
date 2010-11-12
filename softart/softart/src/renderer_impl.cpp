#include "../include/renderer_impl.h"

#include "../include/shaderregs_op.h"
#include "../include/clipper.h"
#include "../include/resource_manager.h"
#include "../include/rasterizer.h"
#include "../include/framebuffer.h"
#include "../include/stream.h"
#include "../include/surface.h"
#include "../include/vertex_cache.h"

BEGIN_NS_SOFTART();

using namespace eflib;

//inherited
result renderer_impl::set_input_layout(const input_layout_decl& layout)
{
	//layout_ 只能到运行期检测了...
	hvertcache_->set_input_layout(layout);
	return result::ok;
}

const input_layout_decl& renderer_impl::get_input_layout() const
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	static input_layout_decl ret;
	return ret;
}

result renderer_impl::set_stream(stream_index sidx, h_buffer hbuf)
{
	hvertcache_->set_stream(stream_index(sidx), hbuf);
	return result::ok;
}

h_buffer renderer_impl::get_stream(stream_index /*sidx*/) const
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return h_buffer();
}

result renderer_impl::set_index_buffer(h_buffer hbuf, index_type idxtype)
{
	switch (idxtype)
	{
	case index_int16:
	case index_int32:
		break;
	default:
		EFLIB_ASSERT(false, "枚举值无效：无效的索引类型");
		return result::failed;
	}

	indexbuf_ = hbuf;
	idxtype_ = idxtype;

	return result::ok;
}

h_buffer renderer_impl::get_index_buffer() const
{
	return indexbuf_;
}

index_type renderer_impl::get_index_type() const
{
	return idxtype_;
}

//
result renderer_impl::set_primitive_topology(primitive_topology primtopo)
{
	switch (primtopo)
	{
	case primitive_line_list:
	case primitive_line_strip:
	case primitive_triangle_list:
	case primitive_triangle_strip:
		break;
	default:
		EFLIB_ASSERT(false, "枚举值无效：无效的图元拓扑枚举。");
		return result::failed;
	}

	primtopo_ = primtopo;
	return result::ok;
}

primitive_topology renderer_impl::get_primitive_topology() const
{
	return primtopo_;
}

result renderer_impl::set_vertex_shader(h_vertex_shader hvs)
{
	hvs_ = hvs;
	vs_output_ops_ = &get_vs_output_op(hvs_->num_output_attributes());
	return result::ok;
}

h_vertex_shader renderer_impl::get_vertex_shader() const
{
	return hvs_;
}

const vs_output_op* renderer_impl::get_vs_output_ops() const
{
	return vs_output_ops_;
}

result renderer_impl::set_rasterizer_state(const h_rasterizer_state& rs)
{
	hrs_ = rs;
	return result::ok;
}

const h_rasterizer_state& renderer_impl::get_rasterizer_state() const
{
	return hrs_;
}

result renderer_impl::set_depth_stencil_state(const h_depth_stencil_state& dss, int32_t stencil_ref)
{
	hdss_ = dss;
	stencil_ref_ = stencil_ref;
	return result::ok;
}

const h_depth_stencil_state& renderer_impl::get_depth_stencil_state() const
{
	return hdss_;
}

int32_t renderer_impl::get_stencil_ref() const
{
	return stencil_ref_;
}

result renderer_impl::set_pixel_shader(h_pixel_shader hps)
{
	hps_ = hps;
	return result::ok;
}

h_pixel_shader renderer_impl::get_pixel_shader() const
{
	return hps_;
}

result renderer_impl::set_blend_shader(h_blend_shader hbs)
{
	hbs_ = hbs;
	return result::ok;
}

h_blend_shader renderer_impl::get_blend_shader()
{
	return hbs_;
}

result renderer_impl::set_viewport(const viewport& vp)
{
	vp_ = vp;
	return result::ok;
}

const viewport& renderer_impl::get_viewport() const
{
	return vp_;
}

result renderer_impl::set_framebuffer_size(size_t width, size_t height, size_t num_samples)
{
	hfb_->reset(width, height, num_samples, hfb_->get_buffer_format());
	return result::ok;
}

eflib::rect<size_t> renderer_impl::get_framebuffer_size() const
{
	return hfb_->get_rect();
}

//
result renderer_impl::set_framebuffer_format(pixel_format pxfmt)
{
	hfb_->reset(hfb_->get_width(), hfb_->get_height(), hfb_->get_num_samples(), pxfmt);
	return result::ok;
}

pixel_format renderer_impl::get_framebuffer_format(pixel_format /*pxfmt*/) const
{
	return hfb_->get_buffer_format();
}

result renderer_impl::set_render_target_available(render_target tar, size_t tar_id, bool valid)
{
	if(valid){
		hfb_->set_render_target_enabled(tar, tar_id);
	} else {
		hfb_->set_render_target_disabled(tar, tar_id);
	}

	return result::ok;
}

bool renderer_impl::get_render_target_available(render_target /*tar*/, size_t /*tar_id*/) const
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return false;
}

//do not support get function for a while
result renderer_impl::set_render_target(render_target tar, size_t tar_id, surface* psurf)
{
	hfb_->set_render_target(tar, tar_id, psurf);
	return result::ok;
}

h_renderer_mementor renderer_impl::create_mementor()
{
	return h_renderer_mementor();
}

result renderer_impl::release_mementor(h_renderer_mementor& /*mementor*/)
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return result::ok;
}

result renderer_impl::set_additional_state(const boost::any& /*state*/)
{
	return result::ok;
}

boost::any renderer_impl::get_additional_state(const boost::any& /*name*/)
{
	return boost::any();
}

//
h_buffer renderer_impl::create_buffer(size_t size)
{
	return hbufmgr_->create_buffer(size);
}

result renderer_impl::release_buffer(h_buffer& hbuf)
{
	hbufmgr_->release_buffer(hbuf);
	return result::ok;
}

h_texture renderer_impl::create_tex2d(size_t width, size_t height, size_t num_samples, pixel_format fmt)
{
	return htexmgr_->create_texture_2d(width, height, num_samples, fmt);
}

h_texture renderer_impl::create_texcube(size_t width, size_t height, size_t num_samples, pixel_format fmt)
{
	return htexmgr_->create_texture_cube(width, height, num_samples, fmt);
}

result renderer_impl::release_texture(h_texture& htex)
{
	htexmgr_->release_texture(htex);
	return result::ok;
}

h_sampler renderer_impl::create_sampler(const sampler_desc& desc)
{
	return h_sampler(new sampler(desc));
}

result renderer_impl::release_sampler(h_sampler& hsmp)
{
	hsmp.reset();
	return result::ok;
}

result renderer_impl::draw(size_t startpos, size_t primcnt)
{
	hrast_->set_state(hrs_);

	hvertcache_->reset(h_buffer(), idxtype_, primtopo_, static_cast<uint32_t>(startpos), 0);
	hvertcache_->transform_vertices(static_cast<uint32_t>(primcnt));
	
	hrast_->draw(primcnt);
	return result::ok;
}

result renderer_impl::draw_index(size_t startpos, size_t primcnt, int basevert)
{
	hrast_->set_state(hrs_);

	hvertcache_->reset(indexbuf_, idxtype_, primtopo_, static_cast<uint32_t>(startpos), basevert);
	hvertcache_->transform_vertices(static_cast<uint32_t>(primcnt));

	hrast_->draw(primcnt);
	return result::ok;
}

result renderer_impl::clear_color(size_t tar_id, const color_rgba32f& c)
{
	hfb_->clear_color(tar_id, c);
	return result::ok;
}

result renderer_impl::clear_depth(float d)
{
	hfb_->clear_depth(d);
	return result::ok;
}

result renderer_impl::clear_stencil(uint32_t s)
{
	hfb_->clear_stencil(s);
	return result::ok;
}

result renderer_impl::clear_color(size_t tar_id, const eflib::rect<size_t>& rc, const color_rgba32f& c)
{
	hfb_->clear_color(tar_id, rc, c);
	return result::ok;
}

result renderer_impl::clear_depth(const eflib::rect<size_t>& rc, float d)
{
	hfb_->clear_depth(rc, d);
	return result::ok;
}

result renderer_impl::clear_stencil(const eflib::rect<size_t>& rc, uint32_t s)
{
	hfb_->clear_stencil(rc, s);
	return result::ok;
}

result renderer_impl::present()
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return result::ok;
}

void renderer_impl::initialize(){
	hrast_->initialize(this);
	hfb_->initialize(this);
}

renderer_impl::renderer_impl(const renderer_parameters* pparam, h_device hdev)
	: idxtype_(index_int16), primtopo_(primitive_triangle_list)
{
	hbufmgr_.reset(new buffer_manager());
	htexmgr_.reset(new texture_manager());

	hclipper_.reset(new clipper());
	hrast_.reset(new rasterizer());
	hfb_.reset(
		new framebuffer(
		pparam->backbuffer_width,
		pparam->backbuffer_height,
		pparam->backbuffer_num_samples,
		pparam->backbuffer_format
		)
		);
	hdev_ = hdev;

	hvertcache_.reset(new default_vertex_cache);
	hvertcache_->initialize(this);

	hrs_.reset(new rasterizer_state(rasterizer_desc()));
	hdss_.reset(new depth_stencil_state(depth_stencil_desc()));

	vp_.minz = 0.0f;
	vp_.maxz = 1.0f;
	vp_.w = static_cast<float>(pparam->backbuffer_width);
	vp_.h = static_cast<float>(pparam->backbuffer_height);
	vp_.x = vp_.y = 0;

	initialize();
}

h_buffer_manager renderer_impl::get_buf_mgr()
{
	return hbufmgr_;
}

h_texture_manager renderer_impl::get_tex_mgr()
{
	return htexmgr_;
}

h_rasterizer renderer_impl::get_rasterizer()
{
	return hrast_;
}

h_framebuffer renderer_impl::get_framebuffer() const
{
	return hfb_; 
}

h_device renderer_impl::get_device()
{
	return hdev_;
}

h_vertex_cache renderer_impl::get_vertex_cache()
{
	return hvertcache_;
}

h_clipper renderer_impl::get_clipper()
{
	return hclipper_;
}

h_renderer create_software_renderer(const renderer_parameters* pparam, h_device hdev)
{
	return h_renderer(new renderer_impl(pparam, hdev));
}
END_NS_SOFTART()
