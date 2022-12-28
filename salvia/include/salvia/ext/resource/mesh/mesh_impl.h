#pragma once

#include "mesh.h"

namespace salvia::ext::resource {

class mesh_impl : public mesh {
public:
  salvia::core::renderer *device_;
  std::shared_ptr<attached_data> attached_;

  // Buffer information
  salvia::resource::buffer_ptr index_buffer_;
  salvia::resource::buffer_ptr adjacancies_;
  std::vector<salvia::resource::buffer_ptr> vertex_buffers_;
  std::vector<size_t> strides_;
  std::vector<size_t> offsets_;
  std::vector<size_t> slots_;

  // Primitive and input descriptions
  size_t primcount_;
  salvia::format index_format_;
  std::vector<salvia::resource::input_element_desc> elem_descs_;
  salvia::resource::input_layout_ptr cached_layout_;

  // Constructor
  explicit mesh_impl(salvia::core::renderer *psr);

  // Implements mesh
  size_t get_buffer_count() override;
  size_t get_face_count() override;
  salvia::resource::buffer_ptr get_index_buffer() override;
  salvia::resource::buffer_ptr get_vertex_buffer(size_t buffer_index) override;
  attached_data_ptr get_attached() override;
  void gen_adjancency() override;
  void render() override;

  // mesh_impl implements
  virtual salvia::resource::buffer_ptr create_buffer(size_t size);
  virtual void set_index_buffer(salvia::resource::buffer_ptr const &);
  virtual void set_index_type(salvia::format index_fmt);
  virtual void add_vertex_buffer(size_t slot, salvia::resource::buffer_ptr const &, size_t stride,
                                 size_t offset);
  virtual void set_primitive_count(size_t primcount);
  virtual void
  set_input_element_descs(const std::vector<salvia::resource::input_element_desc> &descs);
  virtual void set_attached_data(attached_data_ptr const &attached);
};

} // namespace salvia::ext::resource
