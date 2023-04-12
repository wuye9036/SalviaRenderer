#pragma once

#include <eflib/utility/hash.h>

#include <cassert>
#include <string>
#include <utility>

namespace salvia::shader {

enum languages {
  lang_none,

  lang_general,
  lang_vertex_shader,
  lang_pixel_shader,
  lang_blending_shader,

  lang_count
};

enum system_values {
  sv_none,

  sv_position,
  sv_texcoord,
  sv_normal,

  sv_blend_indices,
  sv_blend_weights,
  sv_psize,

  sv_target,
  sv_depth,

  sv_customized
};

class semantic_value {
public:
  static constexpr std::string lower_copy(std::string const& name) {
    std::string ret(name);
    for (auto& ch : ret) {
      if ('A' <= ch && ch <= 'Z') {
        ch -= ('A' - 'a');
      }
    }

    return ret;
  }

  constexpr semantic_value() : sv(sv_none), index(0) {}

  constexpr explicit semantic_value(std::string const& name, uint32_t index = 0) {
    assert(!name.empty());

    std::string lower_name = lower_copy(name);

    if (lower_name == "position" || lower_name == "sv_position") {
      sv = sv_position;
    } else if (lower_name == "normal") {
      sv = sv_normal;
    } else if (lower_name == "texcoord") {
      sv = sv_texcoord;
    } else if (lower_name == "color" || lower_name == "sv_target") {
      sv = sv_target;
    } else if (lower_name == "depth" || lower_name == "sv_depth") {
      sv = sv_depth;
    } else if (lower_name == "blend_indices") {
      sv = sv_blend_indices;
    } else if (lower_name == "blend_weights") {
      sv = sv_blend_weights;
    } else if (lower_name == "psize") {
      sv = sv_psize;
    } else {
      sv = sv_customized;
      this->name = lower_name;
    }
    this->index = index;
  }

  constexpr semantic_value(system_values sv, uint32_t index = 0) {
    assert(sv_none <= sv && sv < sv_customized);
    this->sv = sv;
    this->index = index;
  }

  constexpr std::string const& get_name() const { return name; }

  constexpr system_values get_system_value() const { return sv; }

  constexpr uint32_t get_index() const { return index; }

  constexpr bool operator<(semantic_value const& rhs) const {
    return sv < rhs.sv || name < rhs.name || index < rhs.index;
  }

  constexpr bool operator==(semantic_value const& rhs) const {
    return is_same_sv(rhs) && index == rhs.index;
  }

  constexpr bool operator==(system_values rhs) const { return sv == rhs && index == 0; }

  template <typename T>
  bool operator!=(T const& v) const {
    return *this != v;
  }

  [[nodiscard]] semantic_value advance_index(size_t i) const {
    semantic_value ret;
    ret.name = name;
    ret.sv = sv;
    ret.index = static_cast<uint32_t>(index + i);
    return ret;
  }

  [[nodiscard]] bool valid() const { return sv != sv_none || !name.empty(); }

  [[nodiscard]] float default_w() const { return sv == sv_position ? 1.0f : 0.0f; }

private:
  std::string name;
  system_values sv;
  uint32_t index;

  [[nodiscard]] constexpr bool is_same_sv(semantic_value const& rhs) const noexcept {
    if (sv != rhs.sv)
      return false;
    if (sv == sv_customized)
      return rhs.name == name;
    return true;
  }
};

enum interpolation_modifiers {
  im_none = 0UL,
  im_linear = 1UL << 0,
  im_centroid = 1UL << 1,
  im_nointerpolation = 1UL << 2,
  im_noperspective = 1UL << 3,
  im_sample = 1UL << 4
};

}  // namespace salvia::shader

template <>
struct std::hash<salvia::shader::semantic_value> {
  constexpr size_t operator()(salvia::shader::semantic_value const& sv) const noexcept {
    size_t seed = sv.get_index();
    if (sv.get_system_value() != salvia::shader::sv_customized) {
      eflib::hash_combine(seed, static_cast<size_t>(sv.get_system_value()));
    } else {
      eflib::hash_combine(seed, sv.get_name());
    }
    return seed;
  }
};
