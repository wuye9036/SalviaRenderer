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

result sync_renderer::draw(size_t startpos, size_t primcnt)
{
    state_->cmd = command_id::draw;
	state_->start_index = static_cast<uint32_t>(startpos);
	state_->prim_count  = static_cast<uint32_t>(primcnt);
	state_->base_vertex = 0;

    core_.update(state_);
    core_.execute();

	return result::ok;
}

result sync_renderer::draw_index(size_t startpos, size_t primcnt, int basevert)
{
    state_->cmd = command_id::draw_index;
	state_->start_index = static_cast<uint32_t>(startpos);
	state_->prim_count  = static_cast<uint32_t>(primcnt);
	state_->base_vertex = basevert;

    core_.update(state_);
    core_.execute();

	return result::ok;
}

result sync_renderer::clear_color(surface_ptr const& color_target, color_rgba32f const& c)
{
    state_->clear_color_target = color_target;
    state_->clear_color = c;
    state_->cmd = command_id::clear_color;

    core_.update(state_);
    return core_.execute();
}

result sync_renderer::clear_depth_stencil(surface_ptr const& depth_stencil_target, float d, uint32_t s)
{
    state_->clear_z = d;
    state_->clear_stencil = s;
    state_->clear_ds_target = depth_stencil_target;
    state_->cmd = command_id::clear_depth_stencil;

    core_.update(state_);
    return core_.execute();
}

result sync_renderer::flush()
{
	return result::ok;
}

sync_renderer::sync_renderer()
{
}

renderer_ptr create_sync_renderer()
{
	return renderer_ptr(new sync_renderer());
}

END_NS_SALVIAR();
