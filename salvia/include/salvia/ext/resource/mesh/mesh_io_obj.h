#pragma once

#include <memory>
#include <string>
#include <vector>

namespace salvia::core {
class renderer;
}

namespace salvia::ext::resource {
typedef std::shared_ptr<class mesh> mesh_ptr;
std::vector<mesh_ptr>
create_mesh_from_obj(salvia::core::renderer* render, std::string const& file_name, bool flip_tex_v);
}  // namespace salvia::ext::resource