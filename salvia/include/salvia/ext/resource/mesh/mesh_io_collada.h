#pragma once

#include "eflib/utility/shared_declaration.h"

#include <string>
#include <vector>

namespace salvia::core {
class renderer;
}

namespace salvia::ext::resource {

EFLIB_DECLARE_CLASS_SHARED_PTR(skin_mesh);
EFLIB_DECLARE_CLASS_SHARED_PTR(mesh);

skin_mesh_ptr create_mesh_from_collada(salvia::core::renderer *render, std::string const &file_name);
mesh_ptr create_morph_mesh_from_collada(salvia::core::renderer *render, std::string const &src,
                                        std::string const &dst);

} // namespace salvia::ext::resource
