#include <salviax/include/resource/mesh/sa/skin_mesh_impl.h>

#include <eflib/include/math/math.h>

using eflib::mat44;
using std::vector;
using std::string;

BEGIN_NS_SALVIAX_RESOURCE();

scene_node::scene_node( scene_node* parent, string const& name )
	: parent(parent)
	, name(name)
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
	else
	{
		mat_mul(world_matrix, parent->world_matrix, local_matrix);
	}
}

void scene_node::update_world_matrix_recursive()
{
	update_world_matrix();
	for(scene_node_ptr const& child: children)
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
	for(scene_node_ptr const& child: children)
	{
		child->reset_world_matrix_recursive();
	}
}

void skin_mesh_impl::render( uint32_t submesh_id )
{
	submeshes[submesh_id]->render();
}

vector<mat44> skin_mesh_impl::joint_matrices()
{
	if( joint_mats.size() != joints.size() )
	{
		joint_mats.clear();
		for(size_t i_joint = 0; i_joint < joints.size(); ++i_joint)
		{
			joint_mats.push_back( &(joint_nodes[joints[i_joint]]->world_matrix) );
		}
	}

	for(scene_node_ptr const& pnode: roots)
	{
		pnode->update_world_matrix_recursive();
	}
	
	vector<mat44> ret;
	ret.reserve( joint_mats.size() );
	for(size_t i_joint = 0; i_joint < joints.size(); ++i_joint)
	{
		// mat44 tmp;
		// ret.push_back( eflib::mat_transpose(tmp, *joint_mats[i_joint]) );
		ret.push_back( *joint_mats[i_joint] );
	}

	return ret;
}

void skin_mesh_impl::update_time( float t )
{
	for( animation_player_ptr const& anim_player: anims )
	{
		anim_player->update_play_time(t);
	}
}

void skin_mesh_impl::set_time( float t )
{
	for( animation_player_ptr const& anim_player: anims )
	{
		anim_player->set_play_time(t);
	}
}

size_t skin_mesh_impl::submesh_count()
{
	return submeshes.size();
}

vector<mat44> skin_mesh_impl::bind_inv_matrices()
{
	return bind_inv_mats;
}

END_NS_SALVIAX_RESOURCE();