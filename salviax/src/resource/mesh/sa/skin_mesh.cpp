#include <salviax/include/resource/mesh/sa/skin_mesh.h>

#include <eflib/include/math/math.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/foreach.hpp>
#include <eflib/include/platform/boost_end.h>

using eflib::mat44;
using std::vector;

BEGIN_NS_SALVIAX_RESOURCE();

scene_node::scene_node( scene_node* parent )
	: parent(parent)
	, original_matrix(mat44::identity())
	, local_matrix(mat44::identity())
	, world_matrix(mat44::identity())
{
}

void scene_node::update_world_matrix()
{
	if(parent == NULL)
	{
		world_matrix = local_matrix;
	}
	mat_mul(world_matrix, parent->world_matrix, local_matrix);
}

void scene_node::update_world_matrix_recursive()
{
	update_world_matrix();
	BOOST_FOREACH(scene_node_ptr const& child, children)
	{
		child->update_world_matrix_recursive();
	}
}

void scene_node::reset_world_matrix()
{
	local_matrix = original_matrix;
	update_world_matrix();
}

void scene_node::reset_world_matrix_recursive()
{
	reset_world_matrix();
	BOOST_FOREACH(scene_node_ptr const& child, children)
	{
		child->reset_world_matrix_recursive();
	}
}

void skin_mesh::render( uint32_t submesh_id )
{
	submeshes[submesh_id]->render();
}

vector<mat44> skin_mesh::joint_transformations()
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return vector<mat44>();
}

void skin_mesh::update_time( float t )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
}

void skin_mesh::set_time( float t )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
}

size_t skin_mesh::submesh_count()
{
	return submeshes.size();
}

END_NS_SALVIAX_RESOURCE();