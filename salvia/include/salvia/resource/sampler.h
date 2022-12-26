#pragma once

#include <salvia/common/constants.h>
#include <salvia/resource/colors.h>

#include <eflib/platform/typedefs.h>
#include <eflib/utility/shared_declaration.h>

namespace salvia::resource {

EFLIB_DECLARE_CLASS_SHARED_PTR(texture);
EFLIB_DECLARE_CLASS_SHARED_PTR(texture_1d);
EFLIB_DECLARE_CLASS_SHARED_PTR(texture_2d);
EFLIB_DECLARE_CLASS_SHARED_PTR(surface);

struct sampler_desc {
  filter_type min_filter;
  filter_type mag_filter;
  filter_type mip_filter;
  mip_quality mip_qual;
  address_mode addr_mode_u;
  address_mode addr_mode_v;
  address_mode addr_mode_w;
  float mip_lod_bias;
  uint32_t max_anisotropy;
  compare_function comparison_func;
  color_rgba32f border_color;
  float min_lod;
  float max_lod;

  sampler_desc()
      : min_filter(filter_point), mag_filter(filter_point),
        mip_filter(filter_point), mip_qual{mip_mi_quality}, addr_mode_u(address_wrap),
        addr_mode_v(address_wrap), addr_mode_w(address_wrap), mip_lod_bias(0), max_anisotropy(0),
        comparison_func(compare_function_always),
        border_color(color_rgba32f(0.0f, 0.0f, 0.0f, 0.0f)), min_lod(-1e20f), max_lod(1e20f) {}
};

struct anisotropic_info {
  float lod;
  float probe_count;
  float weight_D;
  eflib::vec4 delta_uv;
};

class sampler {
public:
  typedef color_rgba32f (*filter_op_type)(const surface &surf, float x, float y, size_t sample,
                                          const color_rgba32f &border_color);

private:
  sampler_desc desc_;
  texture_ptr tex_;
  filter_op_type filters_[sampler_state_count];

  float calc_lod(eflib::uint4 const &size, eflib::vec4 const &ddx, eflib::vec4 const &ddy,
                 float bias) const;

  void calc_anisotropic_info(eflib::uint4 const &size, eflib::vec4 const &ddx,
                             eflib::vec4 const &ddy, float bias,
                             anisotropic_info &out_af_info) const;

  color_rgba32f sample_surface(const surface &surf, float x, float y, size_t sample,
                               sampler_state ss) const;

  template <bool IsCubeTexture>
  color_rgba32f sample_impl(int face, float coordx, float coordy, size_t sample, float miplevel,
                            anisotropic_info const *af_info) const;

public:
  explicit sampler(const sampler_desc &desc, texture_ptr const &tex);

  float calc_lod_2d(eflib::vec2 const &ddx, eflib::vec2 const &ddy) const;

  color_rgba32f sample(float coordx, float coordy, float miplevel) const;

  color_rgba32f sample_2d_lod(eflib::vec2 const &proj_coord, float lod) const;

  color_rgba32f sample_2d_grad(eflib::vec2 const &proj_coord, eflib::vec2 const &ddx,
                               eflib::vec2 const &ddy, float lod_bias) const;

  color_rgba32f sample_2d_proj(eflib::vec4 const &proj_coord, eflib::vec4 const &ddx,
                               eflib::vec4 const &ddy) const;

  color_rgba32f sample_cube(float coordx, float coordy, float coordz, float miplevel) const;
};

} // namespace salvia::resource
