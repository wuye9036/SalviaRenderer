#include <salvia/core/clipper.h>
#include <salvia/core/raster_state.h>
#include <salvia/core/rasterizer.h>
#include <salvia/shader/shader_regs.h>

using std::function;

namespace salvia::core {

bool cull_mode_none(float /*area*/) {
  return false;
}

bool cull_mode_ccw(float area) {
  return area <= 0;
}

bool cull_mode_cw(float area) {
  return area >= 0;
}

raster_state::raster_state(const raster_desc& desc) : desc_(desc) {
  switch (desc.cm) {
  case cull_none: cull_ = cull_mode_none; break;

  case cull_front: cull_ = desc.front_ccw ? cull_mode_ccw : cull_mode_cw; break;

  case cull_back: cull_ = desc.front_ccw ? cull_mode_cw : cull_mode_ccw; break;

  default: EFLIB_ASSERT_UNEXPECTED(); break;
  }
}

}  // namespace salvia::core