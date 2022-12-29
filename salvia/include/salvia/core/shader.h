#pragma once

#include <salvia/common/constants.h>
#include <salvia/common/renderer_capacity.h>

#include <salvia/resource/sampler.h>
#include <salvia/shader/constants.h>
#include <salvia/shader/shader_utility.h>

#include <eflib/utility/hash.h>
#include <eflib/utility/shared_declaration.h>

#include <boost/algorithm/string.hpp>

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace salvia::resource {
EFLIB_DECLARE_CLASS_SHARED_PTR(sampler);
struct pixel_accessor;
} // namespace salvia::resource

namespace salvia::shader {

inline size_t hash_value(semantic_value const &v) {
  size_t seed = v.get_index();
  if (v.get_system_value() != sv_customized) {
    eflib::hash_combine(seed, static_cast<size_t>(v.get_system_value()));
  } else {
    eflib::hash_combine(seed, v.get_name());
  }
  return seed;
}

class vs_input;
class vs_output;
struct ps_output;
struct triangle_info;
struct vs_input_op;
struct vs_output_op;

} // namespace salvia::shader

namespace salvia::core {

struct viewport;
struct scanline_info;

EFLIB_DECLARE_CLASS_SHARED_PTR(cpp_shader);

namespace detail = salvia::shader_constant::detail;

shader::vs_input_op &get_vs_input_op(uint32_t n);
shader::vs_output_op &get_vs_output_op(uint32_t n);

float compute_area(const shader::vs_output &v0, const shader::vs_output &v1,
                   const shader::vs_output &v2);
void viewport_transform(eflib::vec4 &position, core::viewport const &vp);

class cpp_shader {
public:
  virtual result set_sampler(const std::string &varname, resource::sampler_ptr const &samp) = 0;
  virtual result set_constant(const std::string &varname, shader_constant::const_voidptr pval) = 0;
  virtual result set_constant(const std::string &varname, shader_constant::const_voidptr pval,
                              size_t index) = 0;

  virtual result find_register(shader::semantic_value const &sv, size_t &index) = 0;
  virtual std::unordered_map<shader::semantic_value, size_t> const &get_register_map() = 0;

  template <typename T> std::shared_ptr<T> clone() {
    auto ret = std::dynamic_pointer_cast<T>(clone());
    assert(ret);
    return ret;
  }

  virtual cpp_shader_ptr clone() = 0;
  virtual ~cpp_shader() = default;
};

class cpp_shader_impl : public cpp_shader {
public:
  result set_sampler(std::string const &samp_name, resource::sampler_ptr const &samp) override {
    auto samp_it = sampmap_.find(samp_name);
    if (samp_it == sampmap_.end()) {
      return result::failed;
    }
    *(samp_it->second) = samp;
    return result::ok;
  }

  result set_constant(const std::string &varname, shader_constant::const_voidptr pval) override {
    auto var_it = varmap_.find(varname);
    if (var_it == varmap_.end()) {
      return result::failed;
    }
    if (shader_constant::assign(var_it->second, pval)) {
      return result::ok;
    }
    return result::failed;
  }

  result set_constant(const std::string &varname, shader_constant::const_voidptr pval,
                      size_t index) override {
    auto cont_it = contmap_.find(varname);
    if (cont_it == contmap_.end()) {
      return result::failed;
    }
    cont_it->second->set(pval, index);
    return result::ok;
  }

  result find_register(shader::semantic_value const &sv, size_t &index) override;
  std::unordered_map<shader::semantic_value, size_t> const &get_register_map() override;
  void bind_semantic(char const *name, size_t semantic_index, size_t register_index);
  void bind_semantic(shader::semantic_value const &s, size_t register_index);

  template <class T> result declare_constant(const std::string &varname, T &var) {
    varmap_[varname] = shader_constant::voidptr(&var);
    return result::ok;
  }

  result declare_sampler(const std::string &varname, resource::sampler_ptr &var) {
    sampmap_[varname] = &var;
    return result::ok;
  }

  template <class T> result declare_container_constant(const std::string &varname, T &var) {
    return declare_container_constant_impl(varname, var, var[0]);
  }

private:
  typedef std::map<std::string, shader_constant::voidptr> variable_map;
  typedef std::map<std::string, resource::sampler_ptr *> sampler_map;
  typedef std::map<std::string, std::shared_ptr<detail::container>> container_variable_map;
  typedef std::unordered_map<shader::semantic_value, size_t> register_map;

  variable_map varmap_;
  container_variable_map contmap_;
  register_map regmap_;
  sampler_map sampmap_;

  template <class T, class ElemType>
  result declare_container_constant_impl(const std::string &varname, T &var, const ElemType &) {
    varmap_[varname] = shader_constant::voidptr(&var);
    contmap_[varname] = std::make_shared<detail::container_impl<T, ElemType>>(var);
    return result::ok;
  }
};

class cpp_vertex_shader : public cpp_shader_impl {
public:
  void execute(const shader::vs_input &in, shader::vs_output &out);
  virtual void shader_prog(const shader::vs_input &in, shader::vs_output &out) = 0;
  virtual uint32_t num_output_attributes() const = 0;
  virtual uint32_t output_attribute_modifiers(uint32_t index) const = 0;
};

using namespace salvia::resource;

class cpp_pixel_shader : public cpp_shader_impl {
  bool front_face_;
  shader::vs_output const *px_;
  shader::vs_output const *quad_;
  uint64_t lod_flag_;
  float lod_[MAX_VS_OUTPUT_ATTRS];

protected:
  bool front_face() const { return front_face_; }

  eflib::vec4 ddx(size_t iReg) const;
  eflib::vec4 ddy(size_t iReg) const;

  color_rgba32f tex2d(const sampler &s, size_t iReg);
  color_rgba32f tex2dlod(const sampler &s, size_t iReg);
  color_rgba32f tex2dlod(sampler const &s, eflib::vec4 const &coord_with_lod);
  color_rgba32f tex2dproj(const sampler &s, size_t iReg);

  color_rgba32f texcube(const sampler &s, const eflib::vec4 &coord, const eflib::vec4 &ddx,
                        const eflib::vec4 &ddy, float bias = 0);
  color_rgba32f texcube(const sampler &s, size_t iReg);
  color_rgba32f texcubelod(const sampler &s, size_t iReg);
  color_rgba32f texcubeproj(const sampler &s, size_t iReg);
  color_rgba32f texcubeproj(const sampler &s, const eflib::vec4 &v, const eflib::vec4 &ddx,
                            const eflib::vec4 &ddy);

public:
  void update_front_face(bool v) { front_face_ = v; }

  uint64_t execute(shader::vs_output const *quad_in, shader::ps_output *px_out, float *depth);

  virtual bool shader_prog(shader::vs_output const &in, shader::ps_output &out) = 0;
  virtual bool output_depth() const;
};

// it is called when render a shaded pixel into framebuffer
class cpp_blend_shader : public cpp_shader_impl {
public:
  void execute(size_t sample, resource::pixel_accessor &inout, const shader::ps_output &in);
  virtual bool shader_prog(size_t sample, resource::pixel_accessor &inout,
                           const shader::ps_output &in) = 0;
};

} // namespace salvia::core
