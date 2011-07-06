#ifndef SALVIAX_MESH_IO_OBJ_H
#define SALVIAX_MESH_IO_OBJ_H

#include <salviax/include/resource/resource_forward.h>

#include <eflib/include/platform/typedefs.h>

#include <salviax/include/resource/mesh/sa/mesh.h>

#include <string>

BEGIN_NS_SALVIAX_RESOURCE();

class obj_material: public attached_data{
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

	~obj_material(){}
};

std::vector<h_mesh> create_mesh_from_obj( std::string const& file_name );

END_NS_SALVIAX_RESOURCE();
#endif