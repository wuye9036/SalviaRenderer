#pragma once

#include <eflib/utility/shared_declaration.h>

#include <memory>
#include <unordered_map>
#include <vector>

namespace LLVM {
class Module;
class IRBuilder;
} // namespace LLVM

namespace salviar {
struct stream_desc;
EFLIB_DECLARE_CLASS_SHARED_PTR(input_layout);
EFLIB_DECLARE_CLASS_SHARED_PTR(shader_reflection);
} // namespace salviar

namespace sasl::shims {

struct ia_shim_key {
  ia_shim_key(salviar::input_layout *input, salviar::shader_reflection const *reflection)
      : input(input), reflection(reflection) {}

  salviar::input_layout *input;
  salviar::shader_reflection const *reflection;

  bool operator==(ia_shim_key const &rhs) const {
    return input == rhs.input && reflection == rhs.reflection;
  }
};

size_t hash_value(ia_shim_key const &);

struct ia_shim_data {
  salviar::stream_desc const *stream_descs;
  intptr_t const *element_offsets; // TODO: OPTIMIZED BY JIT
  size_t const *dest_offsets;      // TODO: OPTIMIZED BY JIT
  size_t count;                    // TODO: OPTIMIZED BY JIT
};

EFLIB_DECLARE_CLASS_SHARED_PTR(ia_shim);
class ia_shim {
public:
  static ia_shim_ptr create();
  virtual ~ia_shim() {}

  virtual void *get_shim_function(std::vector<size_t> &used_slots,
                                  std::vector<intptr_t> &aligned_element_offsets,
                                  std::vector<size_t> &dest_offsets, salviar::input_layout *input,
                                  salviar::shader_reflection const *reflection);
};

} // namespace sasl::shims
