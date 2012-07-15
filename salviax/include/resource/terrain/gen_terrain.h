#ifndef SALVIAX_RESOURCE_GEN_TERRAIN_H
#define SALVIAX_RESOURCE_GEN_TERRAIN_H

#include <salviax/include/resource/resource_forward.h>

#include <eflib/include/utility/shared_declaration.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

#include <vector>

namespace salviar
{
	EFLIB_DECLARE_CLASS_SHARED_PTR(texture);
	class renderer;
}

BEGIN_NS_SALVIAX_RESOURCE();

void make_terrain_random(std::vector<float>& field, int size);
void make_terrain_plasma(std::vector<float>& field, int size, float rough);
void make_terrain_fault (
	std::vector<float>& field, int size,
	int iterations, int max_delta, int min_delta,
	int iterations_per_filter,
	float filter);

void filter_terrain(std::vector<float>& field, int size, float filter);

salviar::texture_ptr make_terrain_texture(salviar::renderer* render, std::vector<float>& normalized_field, int size);

END_NS_SALVIAX_RESOURCE();

#endif