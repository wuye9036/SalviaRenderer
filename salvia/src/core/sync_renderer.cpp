#include <salviar/include/sync_renderer.h>

#include <salviar/include/binary_modules.h>
#include <salvia/shader/shader_regs.h>
#include <salvia/shader/shader_regs_op.h>
#include <salvia/shader/shader_cbuffer.h>
#include <salviar/include/clipper.h>
#include <salviar/include/render_state.h>
#include <salvia/resource/resource_manager.h>
#include <salviar/include/rasterizer.h>
#include <salviar/include/framebuffer.h>
#include <salvia/resource/surface.h>
#include <salviar/include/vertex_cache.h>
#include <salviar/include/stream_assembler.h>
#include <salvia/shader/shader_unit.h>
#include <salvia/resource/input_layout.h>
#include <salviar/include/host.h>
#include <salvia/shader/reflection.h>
#include <salvia/shader/shader_object.h>

namespace salviar{

using namespace eflib;
using std::shared_ptr;

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

}
