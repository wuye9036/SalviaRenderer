#pragma once

#include <eflib/diagnostics/assert.h>
#include <eflib/math/vector.h>

#include <range/v3/algorithm/fold_left.hpp>

#include <algorithm>

namespace eflib {

template <class T> struct rect {
  T x, y, w, h;

  rect() : x(T(0)), y(T(0)), w(T(0)), h(T(0)) {}

  rect(T x, T y, T w, T h) : x(x), y(y), w(w), h(h) {
  }

  template <class U>
  explicit rect(const rect<U> &rhs) : x(T(rhs.x)), y((T)(rhs.y)), w((T)(rhs.w)), h((T)(rhs.h)) {}

  template <class U> rect<T> &operator=(const rect<U> &rhs) {
    x = (T)(rhs.x);
    y = (T)(rhs.y);
    w = (T)(rhs.w);
    h = (T)(rhs.h);
    return *this;
  }

  void set_min(eflib::vec4 &min) {
    x = (T)(min.x());
    y = (T)(min.y());
  }

  void set_max(eflib::vec4 &max) {
    EF_ASSERT(max.x() > x && max.y() > y, "");
    w = (T)(max.x()) - x;
    h = (T)(max.y()) - y;
  }

  eflib::vec4 get_min() { return eflib::vec4((float)x, (float)y, 0.0f, 0.0f); }

  eflib::vec4 get_max() { return eflib::vec4(float(x + w), float(y + h), 0.0f, 0.0f); }

  template <class U> bool is_overlapped(const rect<U> &rc) {
    if (U(rc.x) > x + w)
      return false;
    if (U(rc.y) > y + h)
      return false;
    if (U(rc.x + rc.w) < x)
      return false;
    if (U(rc.y + rc.h) < y)
      return false;
    return true;
  }
};

template <size_t Dimension> class AABB {
public:
  vec4 min_vert;
  vec4 max_vert;

  AABB() = default;

  AABB(const vec4 *vertexes, int n) { set_boundary(vertexes, n); }

  void set_boundary(const vec4 *vertexes, int n) {
    max_vert = min_vert = vertexes[0];
    append_vertex(vertexes + 1, n - 1);
  }

  void append_vertex(const vec4 *vertexes, int n) {
    std::for_each(vertexes, vertexes + n, [this](vec4& vec){
      for (size_t i_dim = 0; i_dim < Dimension; ++i_dim) {
        float& box_min = min_vert[i_dim];
        float& box_max = max_vert[i_dim];
        box_min = std::min(box_min, vec[i_dim]);
        box_max = std::max(box_max, vec[i_dim]);
      }
    });
  }

  bool is_intersect(AABB const& rhs) const {
    for (int i = 0; i < Dimension; ++i) {
      if (min_vert[i] > rhs.max_vert[i])
        return false;
      if (max_vert[i] < rhs.min_vert[i])
        return false;
    }
    return true;
  }

  [[nodiscard]] vec4 get_center() const { return (min_vert + max_vert) / 2.0f; }

  [[nodiscard]] vec4 get_half_size() const { return (max_vert - min_vert) / 2.0f; }

  void get_center_size(vec4 &c, vec4 &hs) const {
    c = get_center();
    hs = max_vert - c;
  }
};

typedef AABB<3> AABB_3D;

bool is_tri_cube_overlap(const AABB_3D &box, const vec4 &v0, const vec4 &v1, const vec4 &v2);
} // namespace eflib