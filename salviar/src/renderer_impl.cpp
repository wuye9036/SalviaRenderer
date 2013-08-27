#include <salviar/include/renderer_impl.h>

#include <salviar/include/binary_modules.h>
#include <salviar/include/shaderregs.h>
#include <salviar/include/shaderregs_op.h>
#include <salviar/include/shader_cbuffer_impl.h>
#include <salviar/include/clipper.h>
#include <salviar/include/render_state.h>
#include <salviar/include/resource_manager.h>
#include <salviar/include/rasterizer.h>
#include <salviar/include/framebuffer.h>
#include <salviar/include/surface.h>
#include <salviar/include/vertex_cache.h>
#include <salviar/include/stream_assembler.h>
#include <salviar/include/shader_unit.h>
#include <salviar/include/input_layout.h>
#include <salviar/include/host.h>
#include <salviar/include/shader_reflection.h>
#include <salviar/include/shader_object.h>

BEGIN_NS_SALVIAR();

using namespace eflib;
using boost::shared_ptr;

result renderer_impl::set_input_layout(const input_layout_ptr& layout)
{
	size_t min_slot = 0, max_slot = 0;
	layout->slot_range(min_slot, max_slot);
	state_->vsi_ops	= &get_vs_input_op( static_cast<uint32_t>(max_slot) );
	state_->layout	= layout;

	return result::ok;
}

result renderer_impl::set_vertex_buffers(
		size_t starts_slot,
		size_t buffers_count, buffer_ptr const* buffers,
		size_t const* strides, size_t const* offsets)
{
	state_->str_state->update(starts_slot, buffers_count, buffers, strides, offsets);
	return result::ok;
}

result renderer_impl::set_index_buffer(buffer_ptr const& hbuf, format index_fmt)
{
	switch (index_fmt)
	{
	case format_r16_uint:
	case format_r32_uint:
		break;
	default:
		EFLIB_ASSERT(false, "The value of index type is invalid.");
		return result::failed;
	}

	state_->index_buffer = hbuf;
	state_->index_format = index_fmt;

	return result::ok;
}

buffer_ptr renderer_impl::get_index_buffer() const{
	return state_->index_buffer;
}

format renderer_impl::get_index_format() const{
	return state_->index_format;
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
		assert( !"Invalid primitive topology." );
		return result::failed;
	}

	state_->prim_topo = primtopo;
	return result::ok;
}

primitive_topology renderer_impl::get_primitive_topology() const{
	return state_->prim_topo;
}

result renderer_impl::set_vertex_shader(cpp_vertex_shader_ptr const& hvs)
{
	state_->cpp_vs = hvs;

	uint32_t n = state_->cpp_vs->num_output_attributes();
	state_->vso_ops = &get_vs_output_op(n);
	for (uint32_t i = 0; i < n; ++ i)
	{
		state_->vso_ops->attribute_modifiers[i] = state_->cpp_vs->output_attribute_modifiers(i);
	}

	return result::ok;
}

cpp_vertex_shader_ptr renderer_impl::get_vertex_shader() const
{
	return state_->cpp_vs;
}

result renderer_impl::set_vertex_shader_code( shared_ptr<shader_object> const& code ){
	state_->vx_shader = code;
	
	state_->vs_proto.reset( new vertex_shader_unit() );
	state_->vs_proto->initialize( state_->vx_shader.get() );

	uint32_t n = state_->vs_proto->output_attributes_count();
	state_->vso_ops = &get_vs_output_op(n);

	for (uint32_t i = 0; i < n; ++ i)
	{
		state_->vso_ops->attribute_modifiers[i] = state_->vs_proto->output_attribute_modifiers(i);
	}

	return result::ok;
}

shared_ptr<shader_object> renderer_impl::get_vertex_shader_code() const{
	return state_->vx_shader;
}

result renderer_impl::set_rasterizer_state(const raster_state_ptr& rs)
{
	state_->ras_state = rs;
	return result::ok;
}

raster_state_ptr renderer_impl::get_rasterizer_state() const
{
	return state_->ras_state;
}

result renderer_impl::set_depth_stencil_state(const depth_stencil_state_ptr& dss, int32_t stencil_ref)
{
	state_->ds_state = dss;
	state_->stencil_ref = stencil_ref;
	return result::ok;
}

const depth_stencil_state_ptr& renderer_impl::get_depth_stencil_state() const
{
	return state_->ds_state;
}

int32_t renderer_impl::get_stencil_ref() const
{
	return state_->stencil_ref;
}

result renderer_impl::set_pixel_shader(cpp_pixel_shader_ptr const& hps)
{
	state_->cpp_ps = hps;
	return result::ok;
}

cpp_pixel_shader_ptr renderer_impl::get_pixel_shader() const
{
	return state_->cpp_ps;
}

result renderer_impl::set_blend_shader(cpp_blend_shader_ptr const& hbs)
{
	state_->cpp_bs = hbs;
	return result::ok;
}

cpp_blend_shader_ptr renderer_impl::get_blend_shader() const
{
	return state_->cpp_bs;
}

result renderer_impl::set_viewport(const viewport& vp)
{
	state_->vp = vp;
	return result::ok;
}

viewport renderer_impl::get_viewport() const
{
	return state_->vp;
}

//do not support get function for a while
result renderer_impl::set_render_target(size_t target_index, surface_ptr const& surf)
{
    if(target_index >= MAX_RENDER_TARGETS)
    {
        return result::failed;
    }
    state_->color_targets.resize(MAX_RENDER_TARGETS);
    state_->color_targets[target_index] = surf;
	return result::ok;
}

result renderer_impl::set_depth_stencil_target(surface_ptr const& surf)
{
    if(surf->get_pixel_format() != pixel_format_color_rg32f)
    {
        return result::failed;
    }
    state_->depth_stencil_target = surf;
    return result::ok;
}

buffer_ptr renderer_impl::create_buffer(size_t size)
{
	return resource_pool_->create_buffer(size);
}

texture_ptr renderer_impl::create_tex2d(size_t width, size_t height, size_t num_samples, pixel_format fmt)
{
	return resource_pool_->create_texture_2d(width, height, num_samples, fmt);
}

texture_ptr renderer_impl::create_texcube(size_t width, size_t height, size_t num_samples, pixel_format fmt)
{
	return resource_pool_->create_texture_cube(width, height, num_samples, fmt);
}

sampler_ptr renderer_impl::create_sampler(const sampler_desc& desc)
{
	return sampler_ptr(new sampler(desc));
}

renderer_impl::renderer_impl(const renderer_parameters* pparam)
    : core_(pparam)
{
	resource_pool_	.reset( new resource_manager() );
	state_			.reset( new render_state() );

	state_->index_format= format_r16_uint;
	state_->prim_topo	= primitive_triangle_list;
}

result renderer_impl::set_vs_variable_value( std::string const& name, void const* var_addr, size_t sz)
{
	state_->vx_cbuffer->set_variable(name, var_addr, sz);
	return result::ok;
}

result renderer_impl::set_vs_variable_pointer( std::string const& name, void const* var_addr, size_t sz )
{
	state_->vx_cbuffer->set_variable(name, var_addr, sz);
	return result::ok;
}

input_layout_ptr renderer_impl::create_input_layout(
	input_element_desc const* elem_descs, size_t elems_count,
	shader_object_ptr const& vs )
{
	return input_layout::create( elem_descs, elems_count, vs );
}

salviar::input_layout_ptr renderer_impl::create_input_layout(
	input_element_desc const* elem_descs, size_t elems_count,
	cpp_vertex_shader_ptr const& vs )
{
	return input_layout::create( elem_descs, elems_count, vs );
}

result renderer_impl::set_pixel_shader_code( shared_ptr<shader_object> const& code )
{
	state_->px_shader = code;
	state_->ps_proto.reset( new pixel_shader_unit() );
	state_->ps_proto->initialize( state_->px_shader.get() );

	return result::ok;
}

shared_ptr<shader_object> renderer_impl::get_pixel_shader_code() const{
	return state_->px_shader;
}

result renderer_impl::set_ps_variable( std::string const& name, void const* data, size_t sz)
{
	state_->px_cbuffer->set_variable(name, data, sz);
	return result::ok;
}

result renderer_impl::set_ps_sampler( std::string const& name, sampler_ptr const& samp )
{
	state_->px_cbuffer->set_sampler(name, samp);
	return result::ok;
}

result renderer_impl::set_vs_sampler( std::string const& name, sampler_ptr const& samp )
{
	state_->vx_cbuffer->set_sampler(name, samp);
	return result::ok;
}

END_NS_SALVIAR();
