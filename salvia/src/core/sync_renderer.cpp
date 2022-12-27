#include <salvia/core/sync_renderer.h>

#include <salvia/resource/input_layout.h>
#include <salvia/resource/resource_manager.h>
#include <salvia/resource/surface.h>
#include <salvia/shader/reflection.h>
#include <salvia/shader/shader_cbuffer.h>
#include <salvia/shader/shader_object.h>
#include <salvia/shader/shader_regs.h>
#include <salvia/shader/shader_regs_op.h>
#include <salvia/core/shader_unit.h>
#include <salvia/core/binary_modules.h>
#include <salvia/core/clipper.h>
#include <salvia/core/framebuffer.h>
#include <salvia/core/host.h>
#include <salvia/core/rasterizer.h>
#include <salvia/core/render_state.h>
#include <salvia/core/stream_assembler.h>
#include <salvia/core/vertex_cache.h>

namespace salvia::core {

using namespace eflib;
using std::shared_ptr;

result sync_renderer::commit_state_and_command() {
  core_.update(state_);
  return core_.execute();
}

result sync_renderer::flush() { return result::ok; }

sync_renderer::sync_renderer() {}

renderer_ptr create_sync_renderer() { return renderer_ptr(new sync_renderer()); }

} // namespace salvia::core
