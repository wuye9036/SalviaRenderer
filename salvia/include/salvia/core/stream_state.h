#pragma once

#include <salvia/common/renderer_capacity.h>

#include <eflib/utility/shared_declaration.h>

#include <array>

namespace salvia::resource {
class buffer;
EFLIB_DECLARE_CLASS_SHARED_PTR(buffer);
}  // namespace salvia::resource

namespace salvia::core {

struct stream_buffer_desc {
  stream_buffer_desc() : slot(0), stride(0), offset(0) {}

  size_t slot;
  resource::buffer_ptr buf;
  size_t stride;
  size_t offset;
};

EFLIB_DECLARE_STRUCT_SHARED_PTR(stream_state);

struct stream_state {
  std::array<stream_buffer_desc, MAX_INPUT_SLOTS> buffer_descs;

  void update(size_t starts_slot,
              size_t buffers_count,
              resource::buffer_ptr const* bufs,
              size_t const* strides,
              size_t const* offsets);
};

}  // namespace salvia::core
