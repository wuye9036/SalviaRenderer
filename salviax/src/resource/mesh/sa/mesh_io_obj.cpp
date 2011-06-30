#include <salviax/include/resource/mesh/sa/mesh_io_obj.h>
#include <fstream>

using std::ifstream;

BEGIN_NS_SALVIAX_RESOURCE();

h_mesh create_mesh_from_obj( std::string const& file_name )
{
	ifstream objf(file_name);

	EFLIB_ASSERT_UNIMPLEMENTED();

	return h_mesh();
}

END_NS_SALVIAX_RESOURCE();