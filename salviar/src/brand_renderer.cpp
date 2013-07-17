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
#include <salviar/include/render_stages.h>
#include <salviar/include/renderer.h>

BEGIN_NS_SALVIAR();

using namespace eflib;
using boost::shared_ptr;

result renderer_impl2::draw(render_state_ptr const& state)
{
	stages_.ras->update(state.get());
	stages_.vert_cache->update( state.get() );
	stages_.vert_cache->transform_vertices(state->prim_count);
	stages_.ras->draw(state->prim_count);

	return result::ok;
}

result renderer_impl2::clear_color(render_state_ptr const& state)
{
	return result::failed;
}

result renderer_impl2::clear_depth(render_state_ptr const& state)
{
	return result::failed;
}

result renderer_impl2::clear_stencil(render_state_ptr const& state)
{
	return result::failed;
}

renderer_impl2::renderer_impl2(renderer_parameters const* params)
{
	// Create stages
	stages_.host		= modules::host::create_host();
	stages_.vert_cache	= create_default_vertex_cache();
	stages_.assembler	.reset( new stream_assembler() );
	stages_.ras			.reset( new rasterizer() );
	stages_.backend		.reset( new framebuffer(params) );

	// Initialize stages
	stages_.vert_cache	->initialize(&stages_);
	stages_.host		->initialize(&stages_);
	stages_.ras			->initialize(&stages_);
	stages_.backend		->initialize(&stages_);
}

END_NS_SALVIAR();
