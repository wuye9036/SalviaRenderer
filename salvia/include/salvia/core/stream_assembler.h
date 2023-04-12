#pragma once

#include <salvia/shader/constants.h>

#include <eflib/utility/shared_declaration.h>

#include <tuple>
#include <unordered_map>
#include <vector>

namespace salvia::resource {
EFLIB_DECLARE_CLASS_SHARED_PTR(buffer);
EFLIB_DECLARE_STRUCT_SHARED_PTR(input_element_desc);
EFLIB_DECLARE_CLASS_SHARED_PTR(input_layout);
}  // namespace salvia::resource

namespace salvia::shader {
class vs_input;
}

namespace salvia::core {
struct render_state;
struct stream_buffer_desc;

struct stream_desc {
  void* buffer;
  size_t offset;
  size_t stride;
};

class stream_assembler {
public:
  virtual ~stream_assembler() {}

  void update(render_state const* state);

  // Used by Cpp Vertex Shader
  void update_register_map(std::unordered_map<shader::semantic_value, size_t> const& reg_map);
  void fetch_vertex(shader::vs_input& vertex, size_t vert_index) const;

  // Used by Old Shader Unit
  void const* element_address(resource::input_element_desc const&, size_t vert_index) const;
  void const* element_address(shader::semantic_value const&, size_t vert_index) const;

  // Used by New Shader Unit
  virtual std::vector<stream_desc> const& get_stream_descs(std::vector<size_t> const& slots);

private:
  // Used by Cpp Vertex Shader
  struct reg_ied_extra_t {
    size_t reg_id;
    resource::input_element_desc const* desc;
    float default_wcomp;
  };
  std::vector<reg_ied_extra_t> reg_ied_extra_;

  // Used by new shader unit
  std::vector<stream_desc> stream_descs_;
  resource::input_layout* layout_;
  stream_buffer_desc const* stream_buffer_descs_;
};

}  // namespace salvia::core
