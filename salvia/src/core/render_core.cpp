#include <salvia/core/render_core.h>

#include <salvia/core/async_object.h>
#include <salvia/core/binary_modules.h>
#include <salvia/core/framebuffer.h>
#include <salvia/core/host.h>
#include <salvia/core/rasterizer.h>
#include <salvia/core/render_stages.h>
#include <salvia/core/shader_unit.h>
#include <salvia/core/stream_assembler.h>
#include <salvia/core/vertex_cache.h>
#include <salvia/resource/input_layout.h>
#include <salvia/resource/resource_manager.h>
#include <salvia/resource/surface.h>
#include <salvia/shader/reflection.h>
#include <salvia/shader/shader_cbuffer.h>
#include <salvia/shader/shader_object.h>
#include <salvia/shader/shader_regs.h>
#include <salvia/shader/shader_regs_op.h>

namespace salvia::core {

using namespace eflib;
using std::shared_ptr;

void render_core::update(render_state_ptr const& state) {
  state_ = state;
}

result render_core::execute() {
  switch (state_->cmd) {
  case command_id::draw:
  case command_id::draw_index: return draw();
  case command_id::clear_color: return clear_color();
  case command_id::clear_depth_stencil: return clear_depth_stencil();
  case command_id::async_begin: return async_start();
  case command_id::async_end: return async_stop();
  }

  ++batch_id_;
  return result::failed;
}

result render_core::draw() {
  if (state_->color_targets.empty() && !state_->depth_stencil_target) {
    return result::ok;
  }

  stages_.assembler->update(state_.get());
  stages_.ras->update(state_.get());
  stages_.vert_cache->update(state_.get());
  if (stages_.host) {
    stages_.host->update(state_.get());
  }
  stages_.backend->update(state_.get());
  apply_shader_cbuffer();

  stages_.ras->draw();

  return result::ok;
}

render_core::render_core() {
  // Create stages
  stages_.host = modules::host::create_host();
  stages_.vert_cache = create_default_vertex_cache();
  stages_.assembler.reset(new stream_assembler());
  stages_.ras.reset(new rasterizer());
  stages_.backend.reset(new framebuffer());

  stages_.vert_cache->initialize(&stages_);
  if (stages_.host) {
    stages_.host->initialize(&stages_);
  }
  stages_.ras->initialize(&stages_);
  stages_.backend->initialize(&stages_);

  batch_id_ = 0;
}

void render_core::apply_shader_cbuffer() {
  if (state_->ps_proto) {
    for (auto const& variable : state_->px_cbuffer.variables()) {
      auto const& var_name = variable.first;
      auto const& var_data = variable.second;
      auto var_data_addr = state_->px_cbuffer.data_pointer(var_data);
      state_->ps_proto->set_variable(var_name, var_data_addr);
    }

    for (auto const& samp : state_->px_cbuffer.samplers()) {
      state_->ps_proto->set_sampler(samp.first, samp.second);
    }
  }
}

result render_core::clear_color() {
  state_->clear_color_target->fill(state_->clear_color);
  return result::ok;
}

result render_core::clear_depth_stencil() {
  if (state_->clear_f == (clear_depth | clear_stencil)) {
    auto ds_color = color_rgba32f(
        state_->clear_z, *reinterpret_cast<float*>(&state_->clear_stencil), 0.0f, 0.0f);
    state_->clear_ds_target->fill(ds_color);
  } else {
    framebuffer::clear_depth_stencil(
        state_->clear_ds_target.get(), state_->clear_f, state_->clear_z, state_->clear_stencil);
  }
  return result::ok;
}

result render_core::async_start() {
  state_->current_async->start_counting();
  return result::ok;
}

result render_core::async_stop() {
  state_->current_async->stop_counting();
  return result::ok;
}

}  // namespace salvia::core
