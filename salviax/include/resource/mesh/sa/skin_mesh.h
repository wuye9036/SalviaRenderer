#ifndef SALVIAX_SKIN_MESH_H
#define SALVIAX_SKIN_MESH_H

#include <salviax/include/resource/resource_forward.h>
#include <eflib/include/metaprog/util.h>
#include <vector>
#include <string>

BEGIN_NS_SALVIAX_RESOURCE();

EFLIB_DECLARE_CLASS_SHARED_PTR(animation_player);
EFLIB_DECLARE_CLASS_SHARED_PTR(joint_node);
EFLIB_DECLARE_CLASS_SHARED_PTR(skin_mesh);

template <typename T>
struct animation_info
{
	enum interpolation_methods{
		im_none,
		im_linear
	};
	
	std::vector<float>					timelines;
	std::vector<T>						values;
	std::vector<interpolation_methods>	interps;
	boost::function<void(T const&)>		applier;
};

class animation_player
{
public:
	virtual void set_play_time(float t) = 0;
	virtual void update_play_time(float elapsed) = 0;
};

template <typename T>
class animation_player_impl : public animation_player
{
public:
	void set_play_time(float t);
	void update_play_time(float elapsed);

private:
	float current_time;
	animation_info<T> anim_info;
};

class joint_node
{
public:
	std::vector<eflib::mat44>	get_joint_matrices( std::vector<std::string> const& names );
	boost::any					get_accessor(std::string const& path);
private:
	std::vector<joint_node_ptr>	children;
	std::string					matrix_name;
	eflib::mat44				matrix;
};

class skin_mesh: public base_skin_mesh
{
private:
	std::vector<h_mesh>			submeshes;
	std::vector<std::string>	joints;
};

END_NS_SALVIAX_RESOURCE();

#endif