#pragma once

#include <salvia/common/constants.h>
#include <salvia/resource/internal_mapped_resource.h>

#include <eflib/diagnostics/assert.h>
#include <eflib/memory/allocator.h>
#include <eflib/platform/typedefs.h>

#include <memory>
#include <vector>


namespace salvia::resource {

class buffer {
  std::vector<uint8_t, eflib::aligned_allocator<uint8_t, 16>> data_;

public:
  buffer(size_t size) : data_(size) {}

  size_t size() const { return data_.size(); }
  uint8_t const *raw_data(size_t offset) const { return data_.data() + offset; }
  uint8_t *raw_data(size_t offset) { return data_.data() + offset; }

  result map(internal_mapped_resource &mapped, map_mode mm);
  result unmap(internal_mapped_resource &mapped, map_mode mm);

  void transfer(size_t offset, void const *psrcdata, size_t size, size_t count);
  void transfer(size_t offset, void const *psrcdata, size_t stride_dest, size_t stride_src,
                size_t size, size_t count);
};

using buffer_ptr = std::shared_ptr<buffer>;

} // namespace salvia::resource
