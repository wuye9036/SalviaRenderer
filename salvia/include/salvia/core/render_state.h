#pragma once

#include "salvia/common/colors.h"
#include "salvia/common/format.h"
#include <salvia/common/constants.h>
#include <salvia/shader/shader_cbuffer.h>

#include <salvia/core/stream_state.h>
#include <salvia/core/viewport.h>

#include <eflib/math/vector.h>
#include <eflib/utility/shared_declaration.h>

namespace salvia::resource {
EFLIB_DECLARE_CLASS_SHARED_PTR(buffer);
EFLIB_DECLARE_CLASS_SHARED_PTR(surface);
EFLIB_DECLARE_CLASS_SHARED_PTR(input_layout);
}  // namespace salvia::resource

namespace salvia::shader {
EFLIB_DECLARE_CLASS_SHARED_PTR(shader_object);
struct vs_input_op;
struct vs_output_op;
}  // namespace salvia::shader

namespace salvia::core {
EFLIB_DECLARE_CLASS_SHARED_PTR(counter);
EFLIB_DECLARE_CLASS_SHARED_PTR(depth_stencil_state);
EFLIB_DECLARE_CLASS_SHARED_PTR(raster_state);
EFLIB_DECLARE_CLASS_SHARED_PTR(cpp_blend_shader);
EFLIB_DECLARE_CLASS_SHARED_PTR(cpp_pixel_shader);
EFLIB_DECLARE_CLASS_SHARED_PTR(cpp_vertex_shader);
EFLIB_DECLARE_CLASS_SHARED_PTR(pixel_shader_unit);
EFLIB_DECLARE_STRUCT_SHARED_PTR(stream_state);
EFLIB_DECLARE_CLASS_SHARED_PTR(async_object);

enum class command_id {
  draw,
  draw_index,
  clear_depth_stencil,
  clear_color,
  async_begin,
  async_end
};

struct render_state {
  command_id cmd;

  resource::buffer_ptr index_buffer;
  format index_format;
  primitive_topology prim_topo;
  int32_t base_vertex;
  uint32_t start_index;
  uint32_t prim_count;

  stream_state str_state;
  resource::input_layout_ptr layout;

  viewport vp;
  raster_state_ptr ras_state;

  int32_t stencil_ref;
  depth_stencil_state_ptr ds_state;

  cpp_vertex_shader_ptr cpp_vs;
  cpp_pixel_shader_ptr cpp_ps;
  cpp_blend_shader_ptr cpp_bs;

  shader::shader_object_ptr vx_shader;
  shader::shader_object_ptr px_shader;

  shader::shader_cbuffer vx_cbuffer;
  shader::shader_cbuffer px_cbuffer;

  shader::vs_input_op* vsi_ops;

  pixel_shader_unit_ptr ps_proto;

  std::vector<resource::surface_ptr> color_targets;
  resource::surface_ptr depth_stencil_target;

  async_object_ptr asyncs[static_cast<int32_t>(async_object_ids::count)];
  async_object_ptr current_async;

  viewport target_vp;
  size_t target_sample_count;

  resource::surface_ptr clear_color_target;
  resource::surface_ptr clear_ds_target;
  uint32_t clear_f;
  float clear_z;
  uint32_t clear_stencil;
  color_rgba32f clear_color;
};

void copy_using_state(render_state* dest, render_state const* src);

}  // namespace salvia::core
