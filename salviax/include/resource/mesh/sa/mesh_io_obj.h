#ifndef SALVIAX_MESH_IO_OBJ_H
#define SALVIAX_MESH_IO_OBJ_H

#include <salviax/include/resource/resource_forward.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/boost_end.h>
#include <vector>
#include <string>

namespace salviar
{
	class renderer;
}

BEGIN_NS_SALVIAX_RESOURCE();
typedef boost::shared_ptr<class mesh> mesh_ptr;
std::vector<mesh_ptr> create_mesh_from_obj( salviar::renderer* render, std::string const& file_name, bool flip_tex_v );
END_NS_SALVIAX_RESOURCE();

#endif