#pragma once

#include <salvia/common/constants.h>

#include <salvia/core/decl.h>
#include <salvia/shader/shader_regs_op.h>

#include <eflib/math/math.h>
#include <eflib/memory/pool.h>

#include <array>
#include <vector>

namespace salvia::core {

const size_t plane_num = 2;

struct clip_context {
  clip_context();

  using vs_output_pool = eflib::pool::reserved_pool<shader::vs_output>;
  using cull_fn = bool (*)(float);

  vs_output_pool *vert_pool;
  prim_type prim;
  shader::vs_output_op const *vso_ops;
  cull_fn cull;
};

struct clip_results {
  shader::vs_output **clipped_verts;
  uint32_t num_clipped_verts;
  bool is_front;
  bool is_clipped;
};

class clipper {
private:
  typedef bool (*cull_fn)(float);
  typedef void (clipper::*clip_impl_fn)(shader::vs_output **, clip_results *);
  typedef eflib::pool::reserved_pool<shader::vs_output> vs_output_pool;

  std::array<eflib::vec4, plane_num> planes_;

  clip_context ctxt_;
  clip_impl_fn clip_impl_;

  void clip_triangle_to_poly_general(shader::vs_output **tri_verts, clip_results *) const;
  void clip_triangle_to_poly_simple(shader::vs_output **tri_verts, clip_results *) const;

  void clip_wireframe_triangle(shader::vs_output **tri_verts, clip_results *rslt);
  void clip_solid_triangle(shader::vs_output **tri_verts, clip_results *rslt);

public:
  clipper();

  void set_context(clip_context const *ctxt);

  inline void clip(shader::vs_output **tri_verts, clip_results *rslt) {
    (this->*clip_impl_)(tri_verts, rslt);
  }
};

} // namespace salvia::core