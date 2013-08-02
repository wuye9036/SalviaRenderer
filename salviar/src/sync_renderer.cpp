#include <salviar/include/sync_renderer.h>

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

//inherited
result sync_renderer::set_input_layout(const input_layout_ptr& layout)
{
	size_t min_slot = 0, max_slot = 0;
	layout->slot_range(min_slot, max_slot);
	state_->vsi_ops	= &get_vs_input_op( static_cast<uint32_t>(max_slot) );
	state_->layout	= layout;

	return result::ok;
}

result sync_renderer::set_vertex_buffers(
		size_t starts_slot,
		size_t buffers_count, buffer_ptr const* buffers,
		size_t const* strides, size_t const* offsets)
{
	state_->str_state->update(starts_slot, buffers_count, buffers, strides, offsets);
	return result::ok;
}

result sync_renderer::set_index_buffer(buffer_ptr const& hbuf, format index_fmt)
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

buffer_ptr sync_renderer::get_index_buffer() const{
	return state_->index_buffer;
}

format sync_renderer::get_index_format() const{
	return state_->index_format;
}

//
result sync_renderer::set_primitive_topology(primitive_topology primtopo)
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

primitive_topology sync_renderer::get_primitive_topology() const{
	return state_->prim_topo;
}

result sync_renderer::set_vertex_shader(cpp_vertex_shader_ptr const& hvs)
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

cpp_vertex_shader_ptr sync_renderer::get_vertex_shader() const
{
	return state_->cpp_vs;
}

result sync_renderer::set_vertex_shader_code( shared_ptr<shader_object> const& code ){
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

shared_ptr<shader_object> sync_renderer::get_vertex_shader_code() const{
	return state_->vx_shader;
}

const vs_input_op* sync_renderer::get_vs_input_ops() const
{
	return state_->vsi_ops;
}

const vs_output_op* sync_renderer::get_vs_output_ops() const
{
	return state_->vso_ops;
}

result sync_renderer::set_rasterizer_state(const raster_state_ptr& rs)
{
	state_->ras_state = rs;
	return result::ok;
}

raster_state_ptr sync_renderer::get_rasterizer_state() const
{
	return state_->ras_state;
}

result sync_renderer::set_depth_stencil_state(const depth_stencil_state_ptr& dss, int32_t stencil_ref)
{
	state_->ds_state = dss;
	state_->stencil_ref = stencil_ref;
	return result::ok;
}

const depth_stencil_state_ptr& sync_renderer::get_depth_stencil_state() const
{
	return state_->ds_state;
}

int32_t sync_renderer::get_stencil_ref() const
{
	return state_->stencil_ref;
}

result sync_renderer::set_pixel_shader(cpp_pixel_shader_ptr const& hps)
{
	state_->cpp_ps = hps;
	return result::ok;
}

cpp_pixel_shader_ptr sync_renderer::get_pixel_shader() const
{
	return state_->cpp_ps;
}

result sync_renderer::set_blend_shader(cpp_blend_shader_ptr const& hbs)
{
	state_->cpp_bs = hbs;
	return result::ok;
}

cpp_blend_shader_ptr sync_renderer::get_blend_shader() const
{
	return state_->cpp_bs;
}

result sync_renderer::set_viewport(const viewport& vp)
{
	state_->vp = vp;
	return result::ok;
}

viewport sync_renderer::get_viewport() const
{
	return state_->vp;
}

result sync_renderer::set_framebuffer_size(size_t width, size_t height, size_t num_samples)
{
	stages_.backend->reset(width, height, num_samples, stages_.backend->get_buffer_format());
	return result::ok;
}

eflib::rect<size_t> sync_renderer::get_framebuffer_size() const
{
	return stages_.backend->get_rect();
}

//
result sync_renderer::set_framebuffer_format(pixel_format pxfmt)
{
	stages_.backend->reset(stages_.backend->get_width(), stages_.backend->get_height(), stages_.backend->get_num_samples(), pxfmt);
	return result::ok;
}

pixel_format sync_renderer::get_framebuffer_format(pixel_format /*pxfmt*/) const
{
	return stages_.backend->get_buffer_format();
}

//do not support get function for a while
result sync_renderer::set_render_target(render_target tar, size_t target_index, surface_ptr const& surf)
{
	stages_.backend->set_render_target( tar, target_index, surf.get() );
	return result::ok;
}

buffer_ptr sync_renderer::create_buffer(size_t size)
{
	return resource_pool_->create_buffer(size);
}

texture_ptr sync_renderer::create_tex2d(size_t width, size_t height, size_t num_samples, pixel_format fmt)
{
	return resource_pool_->create_texture_2d(width, height, num_samples, fmt);
}

texture_ptr sync_renderer::create_texcube(size_t width, size_t height, size_t num_samples, pixel_format fmt)
{
	return resource_pool_->create_texture_cube(width, height, num_samples, fmt);
}

sampler_ptr sync_renderer::create_sampler(const sampler_desc& desc)
{
	return sampler_ptr(new sampler(desc));
}

result sync_renderer::draw(size_t startpos, size_t primcnt)
{
	state_->start_index = static_cast<uint32_t>(startpos);
	state_->prim_count  = static_cast<uint32_t>(primcnt);
	state_->base_vertex = 0;

	// TODO just save/restore index buffer in current version.
	auto current_ib = state_->index_buffer;
	state_->index_buffer.reset();

	stages_.assembler->update(state_.get());
	stages_.ras->update(state_.get());
	stages_.vert_cache->update(state_.get());
	stages_.host->update(state_.get());
	
	apply_shader_cbuffer();

	stages_.vert_cache->transform_vertices(static_cast<uint32_t>(primcnt));
	stages_.ras->draw(primcnt);
	
	state_->index_buffer = current_ib;
	return result::ok;
}

result sync_renderer::draw_index(size_t startpos, size_t primcnt, int basevert)
{
	state_->start_index = static_cast<uint32_t>(startpos);
	state_->prim_count  = static_cast<uint32_t>(primcnt);
	state_->base_vertex = basevert;

	// TODO just save/restore index buffer in current version.
	stages_.assembler->update(state_.get());
	stages_.ras	->update( state_.get() );
	stages_.vert_cache->update( state_.get() );
	stages_.backend->update( state_.get() );
	stages_.host->update(state_.get());
	
	apply_shader_cbuffer();

	stages_.vert_cache->transform_vertices(static_cast<uint32_t>(primcnt));
	stages_.ras->draw(primcnt);

	return result::ok;
}

result sync_renderer::clear_color(size_t target_index, const color_rgba32f& c)
{
	stages_.backend->clear_color(target_index, c);
	return result::ok;
}

result sync_renderer::clear_depth(float d)
{
	stages_.backend->clear_depth(d);
	return result::ok;
}

result sync_renderer::clear_stencil(uint32_t s)
{
	stages_.backend->clear_stencil(s);
	return result::ok;
}

result sync_renderer::clear_color(size_t target_index, const eflib::rect<size_t>& rc, const color_rgba32f& c)
{
	stages_.backend->clear_color(target_index, rc, c);
	return result::ok;
}

result sync_renderer::clear_depth(const eflib::rect<size_t>& rc, float d)
{
	stages_.backend->clear_depth(rc, d);
	return result::ok;
}

result sync_renderer::clear_stencil(const eflib::rect<size_t>& rc, uint32_t s)
{
	stages_.backend->clear_stencil(rc, s);
	return result::ok;
}

result sync_renderer::flush()
{
	return result::ok;
}

result sync_renderer::present()
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return result::ok;
}

void sync_renderer::initialize()
{
	// stages_.assembler->initialize(stages_);
	stages_.vert_cache->initialize(&stages_);
	stages_.host->initialize(&stages_);
	stages_.ras->initialize(&stages_);
	stages_.backend->initialize(&stages_);
}

sync_renderer::sync_renderer(const renderer_parameters* pparam)
{
	resource_pool_	.reset( new resource_manager() );
	state_			.reset( new render_state() );

	state_->index_format= format_r16_uint;
	state_->prim_topo	= primitive_triangle_list;
	
	stages_.host = modules::host::create_host();
	stages_.assembler.reset(new stream_assembler() );
	stages_.ras.reset(new rasterizer());
	stages_.backend.reset(
		new framebuffer(
		pparam->backbuffer_width,
		pparam->backbuffer_height,
		pparam->backbuffer_num_samples,
		pparam->backbuffer_format
		)
		);
	
	stages_.vert_cache = create_default_vertex_cache();
	state_->str_state.reset(new stream_state());
	state_->ras_state.reset(new raster_state(raster_desc()));
	state_->ds_state.reset(new depth_stencil_state(depth_stencil_desc()));
	state_->vx_cbuffer.reset(new shader_cbuffer_impl());
	state_->px_cbuffer.reset(new shader_cbuffer_impl());

	state_->vp.minz = 0.0f;
	state_->vp.maxz = 1.0f;
	state_->vp.w = static_cast<float>(pparam->backbuffer_width);
	state_->vp.h = static_cast<float>(pparam->backbuffer_height);
	state_->vp.x = state_->vp.y = 0;

	initialize();
}

framebuffer_ptr sync_renderer::get_framebuffer() const
{
	return stages_.backend; 
}

result sync_renderer::set_vs_variable_value( std::string const& name, void const* var_addr, size_t sz)
{
	state_->vx_cbuffer->set_variable(name, var_addr, sz);
	return result::ok;
}

result sync_renderer::set_vs_variable_pointer( std::string const& name, void const* var_addr, size_t sz )
{
	state_->vx_cbuffer->set_variable(name, var_addr, sz);
	return result::ok;
}

shared_ptr<vertex_shader_unit> sync_renderer::vs_proto() const{
	return state_->vs_proto;
}

input_layout_ptr sync_renderer::create_input_layout(
	input_element_desc const* elem_descs, size_t elems_count,
	shader_object_ptr const& vs )
{
	return input_layout::create( elem_descs, elems_count, vs );
}

salviar::input_layout_ptr sync_renderer::create_input_layout(
	input_element_desc const* elem_descs, size_t elems_count,
	cpp_vertex_shader_ptr const& vs )
{
	return input_layout::create( elem_descs, elems_count, vs );
}

result sync_renderer::set_pixel_shader_code( shared_ptr<shader_object> const& code )
{
	state_->px_shader = code;
	state_->ps_proto.reset( new pixel_shader_unit() );
	state_->ps_proto->initialize( state_->px_shader.get() );

	return result::ok;
}

shared_ptr<shader_object> sync_renderer::get_pixel_shader_code() const{
	return state_->px_shader;
}

shared_ptr<pixel_shader_unit> sync_renderer::ps_proto() const
{
	return state_->ps_proto;
}

result sync_renderer::set_ps_variable( std::string const& name, void const* data, size_t sz)
{
	state_->px_cbuffer->set_variable(name, data, sz);
	return result::ok;
}

result sync_renderer::set_ps_sampler( std::string const& name, sampler_ptr const& samp )
{
	state_->px_cbuffer->set_sampler(name, samp);
	return result::ok;
}

result sync_renderer::set_vs_sampler( std::string const& name, sampler_ptr const& samp )
{
	state_->vx_cbuffer->set_sampler(name, samp);
	return result::ok;
}

void sync_renderer::apply_shader_cbuffer()
{
	if(state_->vs_proto)
	{
		for(auto const& variable: state_->vx_cbuffer->variables())
		{
			auto const& var_name = variable.first;
			auto const& var_data = variable.second;
			auto var_data_addr = state_->vx_cbuffer->data_pointer(var_data);

			sv_layout* layout = state_->vx_shader->get_reflection()->input_sv_layout(var_name);
			if(layout->agg_type == aggt_array)
			{
				state_->vs_proto->set_variable_pointer(var_name, var_data_addr, var_data.length);
			}
			else
			{
				state_->vs_proto->set_variable(var_name, var_data_addr);
			}
		}

		for(auto const& samp: state_->vx_cbuffer->samplers())
		{
			state_->vs_proto->set_sampler(samp.first, samp.second);
		}
	}

	if(state_->ps_proto)
	{
		for(auto const& variable: state_->px_cbuffer->variables())
		{
			auto const& var_name = variable.first;
			auto const& var_data = variable.second;
			auto var_data_addr = state_->px_cbuffer->data_pointer(var_data);
			state_->ps_proto->set_variable(var_name, var_data_addr);
		}

		for(auto const& samp: state_->px_cbuffer->samplers())
		{
			state_->ps_proto->set_sampler(samp.first, samp.second);
		}
	}
}

renderer_ptr create_sync_renderer(renderer_parameters const* pparam)
{
	return renderer_ptr(new sync_renderer(pparam));
}

END_NS_SALVIAR();
