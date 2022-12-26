#ifndef SALVIAX_MESH_IO_OBJ_H
#define SALVIAX_MESH_IO_OBJ_H

#include <salviax/include/resource/resource_forward.h>

#include <vector>
#include <string>
#include <memory>

namespace salviar
{
	class renderer;
}

namespace salviax::resource{
typedef std::shared_ptr<class mesh> mesh_ptr;
std::vector<mesh_ptr> create_mesh_from_obj( salviar::renderer* render, std::string const& file_name, bool flip_tex_v );
}

#endif