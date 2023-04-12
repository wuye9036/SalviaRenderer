#pragma once

#include <salvia/common/colors.h>
#include <salvia/common/colors_convertors.h>

#include <eflib/math/collision_detection.h>

#include <FreeImage.h>

#include <algorithm>

namespace salvia::ext::utility {

FIBITMAP* load_image(const std::string& fname, int flag FI_DEFAULT(0));

// Return true if image type is supported for loading.
bool check_image_type_support(FIBITMAP* image);

// Stretch region of bitmap to another region with specified size.
FIBITMAP* make_bitmap_copy(eflib::rect<size_t>& out_region,
                           size_t dest_width,
                           size_t dest_height,
                           FIBITMAP* image,
                           const eflib::rect<size_t>& src_region);

// Make all colors with different byte orders to the same interface
// to access color components.
template <class ColorType>
struct FREE_IMAGE_UNIFORM_COLOR {};

template <>
struct FREE_IMAGE_UNIFORM_COLOR<RGBQUAD> {
  using CompT = uint8_t;
  const CompT& r;
  const CompT& g;
  const CompT& b;
  const CompT& a;
  FREE_IMAGE_UNIFORM_COLOR(const CompT* c, CompT /*alpha*/)
    : r(c[FI_RGBA_RED])
    , g(c[FI_RGBA_GREEN])
    , b(c[FI_RGBA_BLUE])
    , a(c[FI_RGBA_ALPHA]) {}

private:
  FREE_IMAGE_UNIFORM_COLOR(const FREE_IMAGE_UNIFORM_COLOR<RGBQUAD>&) = delete;
  FREE_IMAGE_UNIFORM_COLOR& operator=(const FREE_IMAGE_UNIFORM_COLOR<RGBQUAD>&) = delete;
};

template <>
struct FREE_IMAGE_UNIFORM_COLOR<RGBTRIPLE> {
  using CompT = uint8_t;
  const CompT& r;
  const CompT& g;
  const CompT& b;
  const CompT a;
  FREE_IMAGE_UNIFORM_COLOR(const CompT* c, CompT alpha)
    : r(c[FI_RGBA_RED])
    , g(c[FI_RGBA_GREEN])
    , b(c[FI_RGBA_BLUE])
    , a(alpha) {}

private:
  FREE_IMAGE_UNIFORM_COLOR(const FREE_IMAGE_UNIFORM_COLOR<RGBTRIPLE>&) = delete;
  FREE_IMAGE_UNIFORM_COLOR& operator=(const FREE_IMAGE_UNIFORM_COLOR<RGBTRIPLE>&) = delete;
};

template <>
struct FREE_IMAGE_UNIFORM_COLOR<FIRGBF> {
  using CompT = float;
  const CompT& r;
  const CompT& g;
  const CompT& b;
  const CompT a;
  FREE_IMAGE_UNIFORM_COLOR(const CompT* c, CompT alpha) : r(c[0]), g(c[1]), b(c[2]), a(alpha) {}

private:
  FREE_IMAGE_UNIFORM_COLOR(const FREE_IMAGE_UNIFORM_COLOR<FIRGBF>&) = delete;
  FREE_IMAGE_UNIFORM_COLOR& operator=(const FREE_IMAGE_UNIFORM_COLOR<FIRGBF>&) = delete;
};

template <>
struct FREE_IMAGE_UNIFORM_COLOR<FIRGBAF> {
  using CompT = float;
  const CompT& r;
  const CompT& g;
  const CompT& b;
  const CompT& a;
  FREE_IMAGE_UNIFORM_COLOR(const CompT* c, CompT /*alpha*/) : r(c[0]), g(c[1]), b(c[2]), a(c[3]) {}

private:
  FREE_IMAGE_UNIFORM_COLOR(const FREE_IMAGE_UNIFORM_COLOR<FIRGBAF>&) = delete;
  FREE_IMAGE_UNIFORM_COLOR& operator=(const FREE_IMAGE_UNIFORM_COLOR<FIRGBAF>&) = delete;
};

#define FIUC FREE_IMAGE_UNIFORM_COLOR

// Get the salvia supported color type
// which is compatible with internal color format in FreeImage
// but components order of it is RGBA.
template <typename FIColorT>
struct salvia_rgba_color_type {
  typedef salvia::color_max type;
  static const salvia::pixel_format fmt = salvia::pixel_type_to_fmt<type>::fmt;
};
template <>
struct salvia_rgba_color_type<RGBQUAD> {
  typedef salvia::color_rgba8 type;
  static const salvia::pixel_format fmt = salvia::pixel_type_to_fmt<type>::fmt;
};

template <>
struct salvia_rgba_color_type<RGBTRIPLE> {
  typedef salvia::color_rgba8 type;
  static const salvia::pixel_format fmt = salvia::pixel_type_to_fmt<type>::fmt;
};

template <>
struct salvia_rgba_color_type<FIRGBF> {
  typedef salvia::color_rgba32f type;
  static const salvia::pixel_format fmt = salvia::pixel_type_to_fmt<type>::fmt;
};

template <>
struct salvia_rgba_color_type<FIRGBAF> {
  typedef salvia::color_rgba32f type;
  static const salvia::pixel_format fmt = salvia::pixel_type_to_fmt<type>::fmt;
};

static salvia_rgba_color_type<FIRGBAF> color_type_instance;

}  // namespace salvia::ext::utility
