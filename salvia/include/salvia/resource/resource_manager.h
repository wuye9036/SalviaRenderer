#pragma once

#include <salvia/common/constants.h>
#include <salvia/resource/buffer.h>
#include <salvia/resource/internal_mapped_resource.h>
#include <salvia/resource/texture.h>

#include <eflib/memory/allocator.h>

#include <vector>
#include <memory>

namespace salvia::resource {

struct mapped_resource;

class resource_manager {
public:
  resource_manager(std::function<void()> sync)
      : renderer_sync_(sync), map_mode_(map_mode_none),
        mapped_resource_([this](size_t sz) -> void * { return this->reallocate_buffer(sz); }) {}

  buffer_ptr create_buffer(size_t size) { return std::make_shared<buffer>(size); }

  texture_ptr create_texture_2d(size_t width, size_t height, size_t num_samples, pixel_format fmt) {
    return std::make_shared<texture_2d>(width, height, num_samples, fmt);
  }

  texture_ptr create_texture_cube(size_t width, size_t height, size_t num_samples,
                                  pixel_format fmt) {
    return std::make_shared<texture_cube>(width, height, num_samples, fmt);
  }

  result map(mapped_resource &, buffer_ptr const &buf, map_mode mm);
  result map(mapped_resource &, surface_ptr const &surf, map_mode mm);

  result unmap();

private:
  template <typename T> result map_impl(mapped_resource &, T const &res, map_mode mm);
  void *reallocate_buffer(size_t sz);

  std::function<void()> renderer_sync_; // Synchronize/Flush with renderer command queue.
  std::vector<uint8_t, eflib::aligned_allocator<uint8_t, 16>>
      mapped_data_; // Temporary mapped data.
  internal_mapped_resource mapped_resource_;
  map_mode map_mode_;
  surface_ptr mapped_surf_;
  buffer_ptr mapped_buf_;
};

} // namespace salvia::resource
