#include <salviar/include/brand_renderer.h>

#include <salviar/include/binary_modules.h>
#include <salviar/include/shaderregs.h>
#include <salviar/include/shaderregs_op.h>
#include <salviar/include/clipper.h>
#include <salviar/include/resource_manager.h>
#include <salviar/include/rasterizer.h>
#include <salviar/include/framebuffer.h>
#include <salviar/include/surface.h>
#include <salviar/include/vertex_cache.h>
#include <salviar/include/stream_assembler.h>
#include <salviar/include/shader_unit.h>
#include <salviar/include/input_layout.h>
#include <salviar/include/host.h>

BEGIN_NS_SALVIAR();

using namespace eflib;
using boost::shared_ptr;

result renderer_impl2::draw(render_state_ptr const& state)
{
	state_ = state;

	vertex_cache_->update(state.get());
	vertex_cache_->transform_vertices(state_->prim_count);
	
	/*
	rast_->update(state);
	rast_->draw();
	*/

	return result::ok;
}

result renderer_impl2::clear_color(size_t target_index, const color_rgba32f& c)
{
	frame_buffer_->clear_color(target_index, c);
	return result::ok;
}

result renderer_impl2::clear_depth(float d)
{
	frame_buffer_->clear_depth(d);
	return result::ok;
}

result renderer_impl2::clear_stencil(uint32_t s)
{
	frame_buffer_->clear_stencil(s);
	return result::ok;
}

renderer_impl2::renderer_impl2()
{
	host_			= modules::host::create_host();
	vertex_cache_	= create_default_vertex_cache();

	assembler_		.reset(new stream_assembler() );
	clipper_		.reset(new clipper());
	rast_			.reset(new rasterizer());
	frame_buffer_	.reset();
}

END_NS_SALVIAR();
