#pragma once

#include <functional>

namespace salvia::resource {

struct internal_mapped_resource {
  internal_mapped_resource(std::function<void *(size_t)> realloc) : reallocator(realloc) {}
  void *data;
  uint32_t row_pitch;
  uint32_t depth_pitch;

  std::function<void *(size_t)> reallocator;
};

} // namespace salviar
