#include <sasl/shims/ia_shim.h>

#include <salvia/core/shader.h>
#include <salvia/core/stream_assembler.h>
#include <salvia/resource/input_layout.h>
#include <salvia/shader/reflection.h>

#include <eflib/diagnostics/assert.h>
#include <eflib/utility/hash.h>

#include <utility>
#include <vector>

using namespace salvia::core;
using namespace salvia::shader;
using std::make_pair;
using std::vector;

namespace sasl::shims {

size_t hash_value(ia_shim_key const& key) {
  size_t seed = 0;

  eflib::hash_combine(seed, key.input);       // Input layout we need to hash all fields.
  eflib::hash_combine(seed, key.reflection);  // Shader layout we just use address as hash code.

  return seed;
}

ia_shim_ptr ia_shim::create() {
  return ia_shim_ptr(new ia_shim());
}

void common_ia_shim(void* output_buffer, ia_shim_data const* mapping, size_t ivert);

void* ia_shim::get_shim_function(std::vector<size_t>& used_slots,
                                 std::vector<intptr_t>& aligned_element_offsets,
                                 std::vector<size_t>& dest_offsets,
                                 input_layout* input,
                                 shader_reflection const* reflection) {
  // Compute output address and input slots.
  vector<sv_layout*> layouts = reflection->layouts(su_stream_in);

  used_slots.clear();
  aligned_element_offsets.clear();
  dest_offsets.clear();

  for (auto layout : layouts) {
    input_element_desc const* element_desc = input->find_desc(layout->sv);
    used_slots.push_back(element_desc->input_slot);
    aligned_element_offsets.push_back(element_desc->aligned_byte_offset);
    dest_offsets.push_back(layout->offset);
  }

  return (void*)(&common_ia_shim);
}

void common_ia_shim(void* output_buffer, ia_shim_data const* mapping, size_t ivert) {
  uint8_t* output_start = static_cast<uint8_t*>(output_buffer);

  for (size_t i = 0; i < mapping->count; ++i) {
    auto const& str_desc = mapping->stream_descs[i];
    uint8_t* source_start = static_cast<uint8_t*>(str_desc.buffer);
    uint8_t* source_ptr =
        source_start + str_desc.offset + mapping->element_offsets[i] + str_desc.stride * ivert;
    uint8_t* output_addr = output_start + mapping->dest_offsets[i];
    *reinterpret_cast<void**>(output_addr) = source_ptr;
  }
}

}  // namespace sasl::shims
