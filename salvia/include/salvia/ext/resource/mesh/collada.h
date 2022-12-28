#pragma once

#include "salvia/shader/reflection.h"

#include "eflib/math/matrix.h"
#include "eflib/utility/shared_declaration.h"

#include <boost/property_tree/ptree.hpp>

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace salvia::ext::resource {

EFLIB_DECLARE_STRUCT_SHARED_PTR(dae_source);
EFLIB_DECLARE_STRUCT_SHARED_PTR(dae_verts);
EFLIB_DECLARE_STRUCT_SHARED_PTR(dae_array);
EFLIB_DECLARE_STRUCT_SHARED_PTR(dae_tech);
EFLIB_DECLARE_STRUCT_SHARED_PTR(dae_accessor);
EFLIB_DECLARE_STRUCT_SHARED_PTR(dae_triangles);
EFLIB_DECLARE_STRUCT_SHARED_PTR(dae_input);
EFLIB_DECLARE_STRUCT_SHARED_PTR(dae_mesh);
EFLIB_DECLARE_STRUCT_SHARED_PTR(dae_node);
EFLIB_DECLARE_STRUCT_SHARED_PTR(dae_dom);
EFLIB_DECLARE_STRUCT_SHARED_PTR(dae_param);
EFLIB_DECLARE_STRUCT_SHARED_PTR(dae_controller);
EFLIB_DECLARE_STRUCT_SHARED_PTR(dae_skin);
EFLIB_DECLARE_STRUCT_SHARED_PTR(dae_vertex_weights);
EFLIB_DECLARE_STRUCT_SHARED_PTR(dae_animations);
EFLIB_DECLARE_STRUCT_SHARED_PTR(dae_animation);
EFLIB_DECLARE_STRUCT_SHARED_PTR(dae_sampler);
EFLIB_DECLARE_STRUCT_SHARED_PTR(dae_channel);
EFLIB_DECLARE_STRUCT_SHARED_PTR(dae_visual_scenes);
EFLIB_DECLARE_STRUCT_SHARED_PTR(dae_scene_node);
EFLIB_DECLARE_STRUCT_SHARED_PTR(dae_matrix);

struct dae_dom {
  std::unordered_map<std::string, dae_node_ptr> id_nodes;

  template <typename T> std::shared_ptr<T> get_node(std::string const &name) {
    std::string unqual_name = (name[0] == '#' ? name.substr(1) : name);
    auto it = id_nodes.find(unqual_name);
    if (it == id_nodes.end()) {
      return std::shared_ptr<T>();
    }
    return std::dynamic_pointer_cast<T>(it->second);
  }

  template <typename T> void get_node(std::string const &name, std::shared_ptr<T> &ret) {
    ret = get_node<T>(name);
 }

  template <typename T>
  std::shared_ptr<T> load_node(boost::property_tree::ptree &xml_node, dae_node *parent);

  dae_node_ptr node_by_path(std::string const &path);
};

struct dae_node {
  void parse_attribute(boost::property_tree::ptree &xml_node);

  virtual bool parse(boost::property_tree::ptree &xml_node) = 0;

  template <typename T> std::shared_ptr<T> node_by_id(std::string const &name) {
    return owner->get_node<T>(name);
  }

  std::string unqualified_source_name() {
    if (!source)
      return {};
    return ((*source)[0] == '#' ? (*source).substr(1) : (*source));
  }

  dae_node_ptr source_node() {
    if (source) {
      return owner->get_node<dae_node>(*source);
    }
    return {};
  }

  template <typename T> bool is_a() const { return dynamic_cast<T const *>(this) != nullptr; }

  template <typename T> T *as() { return dynamic_cast<T *>(this); }

  template <typename T> T const *as() const { return dynamic_cast<T const *>(this); }

  template <typename T> std::shared_ptr<T> load_child(boost::property_tree::ptree &xml_node) {
    return owner->load_node<T>(xml_node, this);
  }

  boost::optional<std::string> id, sid, name, source;
  dae_dom *owner;
  dae_node *parent;
  std::unordered_map<std::string, dae_node_ptr> sid_children;

  virtual ~dae_node() = default;
};

template <typename T>
std::shared_ptr<T> dae_dom::load_node(boost::property_tree::ptree &xml_node, dae_node *parent) {
  std::shared_ptr<T> ret = std::make_shared<T>();
  ret->owner = this;
  ret->parent = parent;
  ret->parse_attribute(xml_node);
  ret->parse(xml_node);
  if (ret->sid && parent) {
    parent->sid_children.insert(make_pair(*ret->sid, ret));
  }
  if (ret->id) {
    id_nodes.insert(make_pair(*ret->id, ret));
  }

  return ret;
}

struct dae_mesh : public dae_node {
  bool parse(boost::property_tree::ptree &root) override;

  std::vector<dae_source_ptr> sources;
  std::vector<dae_verts_ptr> verts;
  std::vector<dae_triangles_ptr> triangle_sets;
};

struct dae_triangles : public dae_node {
  bool parse(boost::property_tree::ptree &root) override;
  size_t count;
  std::vector<dae_input_ptr> inputs;
  std::vector<int32_t> indexes;
  boost::optional<std::string> material_name;
};

struct dae_verts : public dae_node {
  bool parse(boost::property_tree::ptree &root) override;
  std::vector<dae_input_ptr> inputs;
  size_t count;
  boost::optional<std::string> material_name;
};

struct dae_input : public dae_node {
  bool parse(boost::property_tree::ptree &root) override;
  boost::optional<std::string> semantic;
  size_t offset;
  size_t set;
};

struct dae_source : public dae_node {
  bool parse(boost::property_tree::ptree &root) override;
  dae_array_ptr arr;
  dae_tech_ptr tech;
};

struct dae_array : public dae_node {
  bool parse(boost::property_tree::ptree &root) override;
  void parse_content(std::string const &tag_name);

  enum array_types { none_array, float_array, int_array, name_array, idref_array };

  size_t element_size();

  array_types array_type;
  size_t count;
  boost::optional<std::string> content;

  // Parsed members
  std::vector<int> int_arr;
  std::vector<float> float_arr;
  std::vector<std::string> name_arr;
};

struct dae_param : public dae_node {
  bool parse(boost::property_tree::ptree &root) override;

  bool index(std::string const &index_seq, size_t &idx);
  int index(std::string const &index_seq);
  int index_xyzw_stpq();

  enum special_types { st_none, st_name };

  salvia::shader::language_value_types vtype;
  special_types stype;
};

struct dae_accessor : public dae_node {
  bool parse(boost::property_tree::ptree &root) override;

  size_t offset, stride, count;
  dae_array_ptr source_array;
  std::vector<dae_param_ptr> params;
};

struct dae_tech : public dae_node {
  bool parse(boost::property_tree::ptree &root) override;
  dae_accessor_ptr accessor;
};

struct dae_controller : public dae_node {
  bool parse(boost::property_tree::ptree &root) override;
  dae_skin_ptr skin;
};

struct dae_skin : public dae_node {
  bool parse(boost::property_tree::ptree &root) override;

  eflib::mat44 bind_shape_mat;
  std::vector<dae_source_ptr> joint_sources;
  std::vector<dae_input_ptr> joint_formats;
  dae_vertex_weights_ptr weights;
};

struct dae_vertex_weights : public dae_node {
  bool parse(boost::property_tree::ptree &root) override;
  size_t count;
  std::vector<dae_input_ptr> inputs;
  std::vector<uint32_t> vcount;
  std::vector<uint32_t> v;
};

struct dae_animations : public dae_node {
  bool parse(boost::property_tree::ptree &root) override;
  std::vector<dae_animation_ptr> anims;
};

struct dae_animation : public dae_node {
  bool parse(boost::property_tree::ptree &root) override;
  std::vector<dae_source_ptr> sources;
  dae_sampler_ptr samp;
  dae_channel_ptr channel;
};

struct dae_sampler : public dae_node {
  bool parse(boost::property_tree::ptree &root) override;
  dae_input_ptr data_in, data_out, interpolation;
};

struct dae_channel : public dae_node {
  bool parse(boost::property_tree::ptree &root) override;
  boost::optional<std::string> target;
};

struct dae_visual_scenes : public dae_node {
  bool parse(boost::property_tree::ptree &root) override;
  std::vector<dae_scene_node_ptr> scenes;
};

struct dae_scene_node : public dae_node {
  bool parse(boost::property_tree::ptree &root) override;
  std::vector<dae_scene_node_ptr> children;
  dae_matrix_ptr mat;
  boost::optional<std::string> type_name;
};

struct dae_matrix : public dae_node {
  bool parse(boost::property_tree::ptree &root) override;
  eflib::mat44 mat;
};

} // namespace salvia::ext::resource
