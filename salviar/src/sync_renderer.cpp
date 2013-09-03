#include <salviar/include/sync_renderer.h>

#include <salviar/include/binary_modules.h>
#include <salviar/include/shaderregs.h>
#include <salviar/include/shaderregs_op.h>
#include <salviar/include/shader_cbuffer.h>
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

result sync_renderer::commit_state_and_command()
{
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
