#pragma once

#include <salviar/include/salviar_forward.h>

#include <salviar/include/enums.h>
#include <salviar/include/renderer_capacity.h>
#include <salviar/include/sampler.h>
#include <salviar/include/shader_utility.h>

#include <eflib/utility/hash.h>
#include <eflib/utility/shared_declaration.h>

#include <boost/algorithm/string.hpp>
#include <eflib/platform/boost_begin.h>
#include <eflib/platform/boost_end.h>

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace salviar {


inline size_t hash_value(semantic_value const &v) {
  size_t seed = v.get_index();
  if (v.get_system_value() != sv_customized) {
    eflib::hash_combine(seed, static_cast<size_t>(v.get_system_value()));
  } else {
    eflib::hash_combine(seed, v.get_name());
  }
  return seed;
}

END_NS_SALVIAR()

namespace std {
template <> struct hash<salviar::semantic_value> {
  size_t operator()(salviar::semantic_value const &v) const noexcept {
    size_t seed = v.get_index();
    if (v.get_system_value() != salviar::sv_customized) {
      eflib::hash_combine(seed, static_cast<size_t>(v.get_system_value()));
    } else {
      eflib::hash_combine(seed, v.get_name());
    }
    return seed;
  }
};
}

namespace salviar {

struct viewport;
struct scanline_info;
struct pixel_accessor;
struct triangle_info;
class vs_input;
class vs_output;
struct ps_output;

EFLIB_DECLARE_CLASS_SHARED_PTR(cpp_shader);
EFLIB_DECLARE_CLASS_SHARED_PTR(sampler);

struct shader_profile {
  languages language;
};

class cpp_shader {
public:
  virtual result set_sampler(const std::_tstring &varname,
                             sampler_ptr const &samp) = 0;
  virtual result set_constant(const std::_tstring &varname,
                              shader_constant::const_voidptr pval) = 0;
  virtual result set_constant(const std::_tstring &varname,
                              shader_constant::const_voidptr pval,
                              size_t index) = 0;

  virtual result find_register(semantic_value const &sv, size_t &index) = 0;
  virtual std::unordered_map<semantic_value, size_t> const &
  get_register_map() = 0;

  template <typename T> std::shared_ptr<T> clone() {
    auto ret = std::dynamic_pointer_cast<T>(clone());
    assert(ret);
    return ret;
  }

  virtual cpp_shader_ptr clone() = 0;
  virtual ~cpp_shader() {}
};

class cpp_shader_impl : public cpp_shader {
public:
  result set_sampler(std::_tstring const &samp_name, sampler_ptr const &samp) {
    auto samp_it = sampmap_.find(samp_name);
    if (samp_it == sampmap_.end()) {
      return result::failed;
    }
    *(samp_it->second) = samp;
    return result::ok;
  }

  result set_constant(const std::_tstring &varname,
                      shader_constant::const_voidptr pval) {
    variable_map::iterator var_it = varmap_.find(varname);
    if (var_it == varmap_.end()) {
      return result::failed;
    }
    if (shader_constant::assign(var_it->second, pval)) {
      return result::ok;
    }
    return result::failed;
  }

  result set_constant(const std::_tstring &varname,
                      shader_constant::const_voidptr pval, size_t index) {
    container_variable_map::iterator cont_it = contmap_.find(varname);
    if (cont_it == contmap_.end()) {
      return result::failed;
    }
    cont_it->second->set(pval, index);
    return result::ok;
  }

  result find_register(semantic_value const &sv, size_t &index);
  std::unordered_map<semantic_value, size_t> const &get_register_map();
  void bind_semantic(char const *name, size_t semantic_index,
                     size_t register_index);
  void bind_semantic(semantic_value const &s, size_t register_index);

  template <class T>
  result declare_constant(const std::_tstring &varname, T &var) {
    varmap_[varname] = shader_constant::voidptr(&var);
    return result::ok;
  }

  result declare_sampler(const std::_tstring &varname, sampler_ptr &var) {
    sampmap_[varname] = &var;
    return result::ok;
  }

  template <class T>
  result declare_container_constant(const std::_tstring &varname, T &var) {
    return declare_container_constant_impl(varname, var, var[0]);
  }

private:
  typedef std::map<std::_tstring, shader_constant::voidptr> variable_map;
  typedef std::map<std::_tstring, sampler_ptr *> sampler_map;
  typedef std::map<std::_tstring, std::shared_ptr<detail::container>>
      container_variable_map;
  typedef std::unordered_map<semantic_value, size_t> register_map;

  variable_map varmap_;
  container_variable_map contmap_;
  register_map regmap_;
  sampler_map sampmap_;

  template <class T, class ElemType>
  result declare_container_constant_impl(const std::_tstring &varname, T &var,
                                         const ElemType &) {
    varmap_[varname] = shader_constant::voidptr(&var);
    contmap_[varname] = std::shared_ptr<detail::container>(
        new detail::container_impl<T, ElemType>(var));
    return result::ok;
  }
};

class cpp_vertex_shader : public cpp_shader_impl {
public:
  void execute(const vs_input &in, vs_output &out);
  virtual void shader_prog(const vs_input &in, vs_output &out) = 0;
  virtual uint32_t num_output_attributes() const = 0;
  virtual uint32_t output_attribute_modifiers(uint32_t index) const = 0;
};

class cpp_pixel_shader : public cpp_shader_impl {
  bool front_face_;
  vs_output const *px_;
  vs_output const *quad_;
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

  color_rgba32f texcube(const sampler &s, const eflib::vec4 &coord,
                        const eflib::vec4 &ddx, const eflib::vec4 &ddy,
                        float bias = 0);
  color_rgba32f texcube(const sampler &s, size_t iReg);
  color_rgba32f texcubelod(const sampler &s, size_t iReg);
  color_rgba32f texcubeproj(const sampler &s, size_t iReg);
  color_rgba32f texcubeproj(const sampler &s, const eflib::vec4 &v,
                            const eflib::vec4 &ddx, const eflib::vec4 &ddy);

public:
  void update_front_face(bool v) { front_face_ = v; }

  uint64_t execute(vs_output const *quad_in, ps_output *px_out, float *depth);

  virtual bool shader_prog(vs_output const &in, ps_output &out) = 0;
  virtual bool output_depth() const;
};

// it is called when render a shaded pixel into framebuffer
class cpp_blend_shader : public cpp_shader_impl {
public:
  void execute(size_t sample, pixel_accessor &inout, const ps_output &in);
  virtual bool shader_prog(size_t sample, pixel_accessor &inout,
                           const ps_output &in) = 0;
};

}
