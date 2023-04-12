#include <salvia/resource/surface.h>

namespace salvia::resource {

struct pixel_accessor {
  pixel_accessor(resource::surface** const& color_buffers, resource::surface* ds_buffer) {
    color_buffers_ = color_buffers;
    ds_buffer_ = ds_buffer;
  }

  void set_pos(size_t x, size_t y) {
    x_ = x;
    y_ = y;
  }

  color_rgba32f color(size_t target_index, size_t sample_index) const {
    if (color_buffers_[target_index] == nullptr) {
      return color_rgba32f(0.0f, 0.0f, 0.0f, 0.0f);
    }
    return color_buffers_[target_index]->get_texel(x_, y_, sample_index);
  }

  void color(size_t target_index, size_t sample, const color_rgba32f& clr) {
    if (color_buffers_[target_index] != nullptr) {
      color_buffers_[target_index]->set_texel(x_, y_, sample, clr);
    }
  }

  void* depth_stencil_address(size_t sample) const {
    return ds_buffer_->texel_address(x_, y_, sample);
  }

private:
  pixel_accessor(const pixel_accessor& rhs);
  pixel_accessor& operator=(const pixel_accessor& rhs);

  resource::surface** color_buffers_;
  resource::surface* ds_buffer_;
  size_t x_, y_;
};

}  // namespace salvia::resource