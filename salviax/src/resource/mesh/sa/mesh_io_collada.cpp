#include <salviax/include/resource/mesh/sa/mesh_io_collada.h>

#include <salviar/include/renderer.h>

using std::vector;
using std::string;
using salviar::renderer;

BEGIN_NS_SALVIAX_RESOURCE();

vector<h_mesh> create_mesh_from_collada( renderer* render, std::string const& file_name )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return vector<h_mesh>();
}

END_NS_SALVIAX_RESOURCE();

