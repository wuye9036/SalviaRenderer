#pragma once

#include "mesh.h"

#include "eflib/math/matrix.h"
#include "eflib/utility/shared_declaration.h"

#include <string>
#include <vector>

#include <functional>
#include <memory>

namespace salvia::ext::resource {

EFLIB_DECLARE_CLASS_SHARED_PTR(animation_player);
EFLIB_DECLARE_STRUCT_SHARED_PTR(animation_info);
EFLIB_DECLARE_CLASS_SHARED_PTR(scene_node);
EFLIB_DECLARE_CLASS_SHARED_PTR(skin_mesh_impl);

enum interpolation_methods { im_none, im_linear };

struct animation_info {
  std::vector<interpolation_methods> interps;
};

template <typename T> struct animation_info_impl : public animation_info {
  std::vector<float> X;
  std::vector<T> Y;
  std::function<void(T const &)> applier;
};

class animation_player {
public:
  virtual void set_play_time(float t) = 0;
  virtual void update_play_time(float elapsed) = 0;
  virtual animation_info_ptr anim_info() = 0;
  [[nodiscard]] virtual float anim_length() const = 0;
  virtual ~animation_player() = default;
};

template <typename T> class animation_player_impl : public animation_player {
public:
  animation_player_impl() : aninfo(std::make_shared<animation_info_impl<T>>()), current_time(0) {}

  void set_play_time(float t) override {
    current_time = t;
    aninfo->applier(resolve(current_time));
  }

  void update_play_time(float elapsed) override {
    current_time += elapsed;
    aninfo->applier(resolve(current_time));
  }

  [[nodiscard]] float anim_length() const override {
    if (aninfo->X.empty()) {
      return 0.0f;
    }

    return aninfo->X.back();
  }

  animation_info_ptr anim_info() override { return aninfo; }
  std::shared_ptr<animation_info_impl<T>> anim_info2() { return aninfo; }

private:
  T resolve(float time) {
    // Find segment
    auto it = lower_bound(aninfo->X.begin(), aninfo->X.end(), time);
    size_t i_segment = distance(aninfo->X.begin(), it) - 1;

    if (it == aninfo->X.begin()) {
      switch (aninfo->interps[0]) {
      case im_none:
      case im_linear:
        return aninfo->Y[0];
      }
    }

    if (it == aninfo->X.end()) {
      switch (aninfo->interps[i_segment]) {
      case im_none:
      case im_linear:
        return aninfo->Y.back();
      }
    }

    switch (aninfo->interps[i_segment]) {
    case im_none:
    case im_linear:
      float diff = aninfo->X[i_segment + 1] - aninfo->X[i_segment];
      float t = (time - aninfo->X[i_segment]) / diff;
      return (aninfo->Y[i_segment + 1] - aninfo->Y[i_segment]) * t + aninfo->Y[i_segment];
    }
  }

  std::shared_ptr<animation_info_impl<T>> aninfo;
  float current_time;
};

class scene_node {
public:
  scene_node(scene_node *parent, std::string const &name);

  scene_node *parent;
  std::string name;

  std::vector<scene_node_ptr> children;

  void reset_world_matrix();
  void reset_world_matrix_recursive();

  void update_world_matrix();
  void update_world_matrix_recursive();

  eflib::mat44 local_matrix;
  eflib::mat44 original_matrix;
  eflib::mat44 world_matrix;
};

class skin_mesh_impl : public skin_mesh {
public:
  size_t submesh_count() const override;
  void render(uint32_t submesh_id) override;
  float animation_length() const override;
  void update_time(float t) override;
  void set_time(float t) override;
  std::vector<eflib::mat44> joint_matrices() override;
  std::vector<eflib::mat44> bind_inv_matrices() const override;

public:
  std::unordered_map<std::string, scene_node *> joint_nodes;
  std::vector<eflib::mat44> bind_inv_mats;
  std::vector<animation_player_ptr> anims;
  std::vector<std::string> joints;
  std::vector<eflib::mat44 *> joint_mats;
  std::vector<mesh_ptr> submeshes;
  std::vector<scene_node_ptr> roots;
};

} // namespace salvia::ext::resource
