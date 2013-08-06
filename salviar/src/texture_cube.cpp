#include <salviar/include/texture.h>
#include <salviar/include/surface.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/make_shared.hpp>
#include <eflib/include/platform/boost_end.h>

using boost::make_shared;

BEGIN_NS_SALVIAR();

using namespace eflib;

texture_cube::texture_cube(size_t width, size_t height, size_t num_samples, pixel_format format)
{
	for(size_t i = 0; i < 6; ++i)
	{
		faces_[i] = make_shared<texture_2d>(width, height, num_samples, format);
	}
}

void texture_cube::gen_mipmap(filter_type filter, bool auto_gen)
{
	for(size_t i = 0; i < 6; ++i)
    {
		faces_[i]->gen_mipmap(filter, auto_gen);
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
    size_t min_lod = faces_[0]->get_min_lod();
	return faces_[subresource / min_lod]->get_surface(subresource % min_lod);
}


texture_2d_ptr const& texture_cube::get_face(cubemap_faces face) const
{
	return faces_[face];
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
		faces_[i]->set_max_lod(miplevel);
	}
}

void texture_cube::set_min_lod(size_t miplevel)
{
	for(size_t i = 0; i < 6; ++i)
	{
		faces_[i]->set_min_lod(miplevel);
	}
}

END_NS_SALVIAR();
