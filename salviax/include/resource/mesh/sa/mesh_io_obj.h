#ifndef SALVIAX_MESH_IO_OBJ_H
#define SALVIAX_MESH_IO_OBJ_H

#include <salviax/include/resource/resource_forward.h>
#include <salviax/include/resource/mesh/sa/mesh.h>

#include <string>

BEGIN_NS_SALVIAX_RESOURCE();

class obj_material: public attached_data{
	~obj_material(){}
};

h_mesh create_mesh_from_obj( std::string const& file_name );

END_NS_SALVIAX_RESOURCE();
#endif