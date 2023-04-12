#pragma once

#include "mesh.h"

#include "salvia/resource/texture.h"

#include "eflib/platform/stdint.h"

#include <string>

namespace salvia::ext::resource {

class obj_material : public attached_data {
public:
  obj_material();

  std::string name;

  eflib::vec4 ambient;
  eflib::vec4 diffuse;
  eflib::vec4 specular;

  int shininess;
  float alpha;

  bool is_specular;

  std::string tex_name;
  salvia::resource::texture_ptr tex;
  ~obj_material() {}
};

}  // namespace salvia::ext::resource
