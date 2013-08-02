#include "../include/surface.h"
#include "../include/texture.h"
BEGIN_NS_SALVIAR()

using namespace eflib;

texture_cube::texture_cube(size_t width, size_t height, size_t num_samples, pixel_format format)
	: subtexs_(6, texture_2d(0, 0, num_samples, format))
{
	for(size_t i = 0; i < 6; ++i)
	{
		subtexs_[i].reset(width, height, num_samples, format);
	}
}

void texture_cube::reset(size_t width, size_t height, size_t num_samples, pixel_format format)
{
	new (this) texture_cube(width, height, num_samples, format);
}

void texture_cube::gen_mipmap(filter_type filter, bool auto_gen)
{
	for(size_t i = 0; i < 6; ++i){
		subtexs_[i].gen_mipmap(filter, auto_gen);
	}
}

void texture_cube::map(void** pData, size_t subresource, map_mode mm)
{
	*pData = NULL;
	get_surface(subresource)->map(pData, mm);
}

void texture_cube::unmap(size_t subresource)
{
	get_surface(subresource)->unmap();
}

surface_ptr const& texture_cube::get_surface(size_t subresource) const
{
	return subtexs_[subresource / subtexs_[0].get_min_lod()].get_surface(subresource % subtexs_[0].get_min_lod());
}

texture& texture_cube::get_face(cubemap_faces face){
	return subtexs_[face];
}

const texture& texture_cube::get_face(cubemap_faces face) const{
	return subtexs_[face];
}

size_t texture_cube::get_width(size_t subresource) const
{
	return get_surface(subresource)->get_width();
}

size_t texture_cube::get_height(size_t subresource) const
{
	return get_surface(subresource)->get_height();
}

size_t texture_cube::get_depth(size_t /*subresource*/) const
{
	return 1;
}

size_t texture_cube::get_num_samples(size_t subresource) const
{
	return get_surface(subresource)->get_num_samples();
}

void texture_cube::set_max_lod(size_t miplevel)
{
	for(size_t i = 0; i < 6; ++i)
	{
		subtexs_[i].set_max_lod(miplevel);
	}
}

void texture_cube::set_min_lod(size_t miplevel)
{
	for(size_t i = 0; i < 6; ++i)
	{
		subtexs_[i].set_min_lod(miplevel);
	}
}

END_NS_SALVIAR();
