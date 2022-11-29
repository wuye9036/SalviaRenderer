#ifndef SALVIAX_MESH_IO_COLLADA_H
#define SALVIAX_MESH_IO_COLLADA_H

#include <salviax/include/resource/resource_forward.h>

#include <eflib/include/utility/shared_declaration.h>

#include <vector>
#include <string>

namespace salviar { class renderer; }

namespace salviax::resource{

EFLIB_DECLARE_CLASS_SHARED_PTR(skin_mesh);
EFLIB_DECLARE_CLASS_SHARED_PTR(mesh);

skin_mesh_ptr	create_mesh_from_collada( salviar::renderer* render, std::string const& file_name );
mesh_ptr		create_morph_mesh_from_collada( salviar::renderer* render, std::string const& src, std::string const& dst);

}

#endif