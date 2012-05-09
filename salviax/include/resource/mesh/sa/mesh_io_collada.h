#ifndef SALVIAX_MESH_IO_COLLADA_H
#define SALVIAX_MESH_IO_COLLADA_H

#include <salviax/include/resource/resource_forward.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/boost_end.h>
#include <vector>
#include <string>

namespace salviar { class renderer; }

BEGIN_NS_SALVIAX_RESOURCE();
typedef boost::shared_ptr<class base_mesh> h_mesh;
h_mesh create_mesh_from_collada( salviar::renderer* render, std::string const& file_name );
END_NS_SALVIAX_RESOURCE();

#endif