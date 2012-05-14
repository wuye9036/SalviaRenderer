#ifndef SALVIAX_SKIN_MESH_H
#define SALVIAX_SKIN_MESH_H

#include <salviax/include/resource/resource_forward.h>

BEGIN_NS_SALVIAX_RESOURCE();

class skin_anim_info
{

};

class skin_anim_player
{

};

class skin_mesh: public base_skin_mesh
{
private:
	skin_anim_player_ptr	anim_player;
	vector<h_mesh>			submeshes;
};

END_NS_SALVIAX_RESOURCE();

#endif