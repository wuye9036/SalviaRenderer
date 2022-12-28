#pragma once

#include <salvia/core/renderer.h>
#include <salvia/core/render_core.h>
#include <salvia/core/render_stages.h>

#include <eflib/utility/shared_declaration.h>

namespace salvia::shader {
EFLIB_DECLARE_CLASS_SHARED_PTR(shader_object);
struct vs_input_op;
struct vs_output_op;
}

namespace salvia::core {
EFLIB_DECLARE_CLASS_SHARED_PTR(host);
EFLIB_DECLARE_CLASS_SHARED_PTR(sync_renderer);
EFLIB_DECLARE_CLASS_SHARED_PTR(pixel_shader_unit);
EFLIB_DECLARE_CLASS_SHARED_PTR(stream_assembler);

EFLIB_DECLARE_STRUCT_SHARED_PTR(render_state);

class renderer_impl : public renderer {
protected:
  render_state_ptr state_;
  render_core core_;
  resource_manager_ptr resource_pool_;

public:
  // inherited
  result set_input_layout(input_layout_ptr const &layout) override;
  result set_vertex_buffers(size_t starts_slot, size_t buffers_count,
                                    buffer_ptr const *buffers, size_t const *strides,
                                    size_t const *offsets) override;

  result set_index_buffer(buffer_ptr const &hbuf, format index_fmt) override;
  [[nodiscard]] buffer_ptr get_index_buffer() const override;
  [[nodiscard]] format get_index_format() const override;

  result set_primitive_topology(primitive_topology topology) override;
  [[nodiscard]] primitive_topology get_primitive_topology() const override;

  result set_vertex_shader(cpp_vertex_shader_ptr const &hvs) override;
  [[nodiscard]] cpp_vertex_shader_ptr get_vertex_shader() const override;

  result set_vertex_shader_code(std::shared_ptr<shader::shader_object> const &) override;
  [[nodiscard]] shader::shader_object_ptr get_vertex_shader_code() const override;
  result set_vs_variable_value(std::string const &name, void const *pvariable, size_t sz) override;
  result set_vs_variable_pointer(std::string const &name, void const *pvariable, size_t sz) override;
  result set_vs_sampler(std::string const &name, sampler_ptr const &samp) override;

  result set_rasterizer_state(raster_state_ptr const &rs) override;
  [[nodiscard]] raster_state_ptr get_rasterizer_state() const override;
  result set_depth_stencil_state(depth_stencil_state_ptr const &dss, int32_t stencil_ref) override;
  virtual const depth_stencil_state_ptr &get_depth_stencil_state() const;
  virtual int32_t get_stencil_ref() const;

  result set_pixel_shader(cpp_pixel_shader_ptr const &hps) override;
  [[nodiscard]] cpp_pixel_shader_ptr get_pixel_shader() const override;

  result set_pixel_shader_code(std::shared_ptr<shader::shader_object> const &) override;
  [[nodiscard]] shader::shader_object_ptr get_pixel_shader_code() const override;
  result set_ps_variable(std::string const &name, void const *data, size_t sz) override;
  result set_ps_sampler(std::string const &name, sampler_ptr const &samp) override;

  result set_blend_shader(cpp_blend_shader_ptr const &hbs) override;
  [[nodiscard]] cpp_blend_shader_ptr get_blend_shader() const override;

  result set_viewport(viewport const &vp) override;
  [[nodiscard]] viewport get_viewport() const override;

  result set_render_targets(size_t color_target_count, surface_ptr const *color_targets,
                                    surface_ptr const &ds_target) override;

  result draw(size_t startpos, size_t primcnt) override;
  result draw_index(size_t startpos, size_t primcnt, int basevert) override;
  result clear_color(surface_ptr const &color_target, color_rgba32f const &c) override;
  result clear_depth_stencil(surface_ptr const &depth_stencil_target, uint32_t f, float d,
                                     uint32_t s) override;
  result begin(async_object_ptr const &async_obj) override;
  result end(async_object_ptr const &async_obj) override;
  async_status get_data(async_object_ptr const &async_obj, void *data, bool do_not_wait) override;

  input_layout_ptr create_input_layout(input_element_desc const *elem_descs,
                                               size_t elems_count, shader::shader_object_ptr const &vs) override;
  input_layout_ptr create_input_layout(input_element_desc const *elem_descs,
                                               size_t elems_count, cpp_vertex_shader_ptr const &vs) override;
  buffer_ptr create_buffer(size_t size) override;
  texture_ptr create_tex2d(size_t width, size_t height, size_t num_samples,
                                   pixel_format fmt) override;
  texture_ptr create_texcube(size_t width, size_t height, size_t num_samples,
                                     pixel_format fmt) override;
  sampler_ptr create_sampler(sampler_desc const &desc, texture_ptr const &tex) override;
  async_object_ptr create_query(async_object_ids id) override;

  result map(mapped_resource &, buffer_ptr const &buf, map_mode mm) override;
  result map(mapped_resource &, surface_ptr const &buf, map_mode mm) override;
  result unmap() override;

  renderer_impl();

protected:
  virtual result commit_state_and_command() = 0;
};

} // namespace salvia::core