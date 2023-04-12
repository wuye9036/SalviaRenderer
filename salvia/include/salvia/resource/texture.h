#pragma once

#include <salvia/common/colors.h>
#include <salvia/common/constants.h>
#include <salvia/resource/surface.h>

#include <eflib/utility/shared_declaration.h>

#include <algorithm>
#include <vector>

namespace salvia::resource {

EFLIB_DECLARE_CLASS_SHARED_PTR(texture_2d);
EFLIB_DECLARE_CLASS_SHARED_PTR(texture);

class texture {
protected:
  pixel_format fmt_{};
  size_t sample_count_{};
  size_t min_lod_{};
  size_t max_lod_{};
  eflib::uint4 size_{};
  std::vector<surface_ptr> surfs_;

  static size_t calc_lod_limit(eflib::uint4 sz) {
    size_t rv = 0;
    auto max_sz = std::max(sz[0], std::max(sz[1], sz[2]));
    while (max_sz > 0) {
      max_sz >>= 1;
      ++rv;
    }

    return rv;
  }

public:
  texture() = default;
  virtual ~texture() = default;

  [[nodiscard]] virtual texture_type get_texture_type() const = 0;

  [[nodiscard]] pixel_format format() const { return fmt_; }

  [[nodiscard]] size_t min_lod() const { return min_lod_; }

  [[nodiscard]] size_t max_lod() const { return max_lod_; }

  [[nodiscard]] surface_ptr subresource(size_t index) const {
    if (max_lod_ <= index && index <= min_lod_) {
      return surfs_[index];
    }
    return {};
  }

  [[nodiscard]] surface const* subresource_cptr(size_t index) const {
    if (max_lod_ <= index && index <= min_lod_) {
      return surfs_[index].get();
    }
    return nullptr;
  }

  [[nodiscard]] eflib::uint4 size() const { return size_; }

  [[nodiscard]] eflib::uint4 size(size_t subresource_index) const {
    auto subres = subresource_cptr(subresource_index);
    if (subres) {
      return eflib::uint4(0, 0, 0, 0);
    }
    return subres->size();
  }

  [[nodiscard]] size_t sample_count() const { return sample_count_; }

  void max_lod(int miplevel) { max_lod_ = miplevel; }

  void min_lod(int miplevel) { min_lod_ = miplevel; }

  virtual void gen_mipmap(filter_type filter, bool auto_gen) = 0;
};

class texture_2d : public texture {
public:
  texture_2d(size_t width, size_t height, size_t num_samples, pixel_format format);

  [[nodiscard]] texture_type get_texture_type() const override { return texture_type_2d; };

  void gen_mipmap(filter_type filter, bool auto_gen) override;
};

class texture_cube : public texture {
public:
  texture_cube(size_t width, size_t height, size_t num_samples, pixel_format format);

  [[nodiscard]] texture_type get_texture_type() const override { return texture_type_cube; };

  [[nodiscard]] surface_ptr subresource(size_t face, size_t lod) const {
    return texture::subresource(lod * 6 + face);
  }

  void gen_mipmap(filter_type filter, bool auto_gen) override;
};

}  // namespace salvia::resource
