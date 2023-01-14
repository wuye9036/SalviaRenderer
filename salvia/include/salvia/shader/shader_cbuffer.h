
#pragma once

#include <eflib/utility/shared_declaration.h>

#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace salvia::resource {
EFLIB_DECLARE_CLASS_SHARED_PTR(texture);
EFLIB_DECLARE_CLASS_SHARED_PTR(sampler);
}

namespace salvia::shader {

enum class shader_cdata_type : uint32_t { sdt_none, sdt_pod, sdt_sampler };

struct shader_cdata {
  shader_cdata() : offset(0), length(0), array_size(0) {}

  size_t offset;
  size_t length;
  size_t array_size;
};

class shader_cbuffer {
public:
  virtual void set_sampler(std::string_view name, resource::sampler_ptr const &samp);
  virtual void set_variable(std::string_view name, void const *data, size_t data_length);

  auto const &variables() const { return variables_; }

  auto const &samplers() const { return samplers_; }

  void const *data_pointer(shader_cdata const &cdata) const {
    if (cdata.length == 0) {
      return nullptr;
    }
    return data_memory_.data() + cdata.offset;
  }

  void copy_from(shader_cbuffer const *src) { *this = *src; }

  virtual ~shader_cbuffer() {}

private:
  std::unordered_map<std::string, shader_cdata> variables_;
  std::vector<char> data_memory_;
  std::unordered_map<std::string, resource::sampler_ptr> samplers_;
  std::vector<resource::texture_ptr> textures_;
};

} // namespace salvia::shader
