#pragma once

#include <eflib/utility/shared_declaration.h>

#include <memory>
#include <unordered_map>
#include <vector>

namespace LLVM {
class Module;
class IRBuilder;
}  // namespace LLVM

namespace salvia::core {
struct stream_desc;
}

namespace salvia::resource {
EFLIB_DECLARE_CLASS_SHARED_PTR(input_layout);
}  // namespace salvia::resource

namespace salvia::shader {
EFLIB_DECLARE_CLASS_SHARED_PTR(shader_reflection);
}  // namespace salvia::shader

namespace sasl::shims {

struct ia_shim_key {
  ia_shim_key(salvia::resource::input_layout* input,
              salvia::shader::shader_reflection const* reflection)
    : input(input)
    , reflection(reflection) {}

  salvia::resource::input_layout* input;
  salvia::shader::shader_reflection const* reflection;

  bool operator==(ia_shim_key const& rhs) const {
    return input == rhs.input && reflection == rhs.reflection;
  }
};

size_t hash_value(ia_shim_key const&);

struct ia_shim_data {
  salvia::core::stream_desc const* stream_descs;
  intptr_t const* element_offsets;  // TODO: OPTIMIZED BY JIT
  size_t const* dest_offsets;       // TODO: OPTIMIZED BY JIT
  size_t count;                     // TODO: OPTIMIZED BY JIT
};

EFLIB_DECLARE_CLASS_SHARED_PTR(ia_shim);
class ia_shim {
public:
  static ia_shim_ptr create();
  virtual ~ia_shim() {}

  virtual void* get_shim_function(std::vector<size_t>& used_slots,
                                  std::vector<intptr_t>& aligned_element_offsets,
                                  std::vector<size_t>& dest_offsets,
                                  salvia::resource::input_layout* input,
                                  salvia::shader::shader_reflection const* reflection);
};

}  // namespace sasl::shims
