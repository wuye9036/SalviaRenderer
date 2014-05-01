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
		surfs_.push_back( make_shared<surface>(width, height, num_samples, format) );
	}
}

void texture_cube::gen_mipmap(filter_type filter, bool auto_gen)
{
	if(auto_gen)
    {
		max_lod_ = 0;
		min_lod_ = calc_lod_limit(size_) - 1;
	}

	surfs_.reserve( (min_lod_ + 1) * 6 );

	for(size_t lod_level = max_lod_; lod_level < min_lod_; ++lod_level)
	{
		for(int i_face = 0; i_face < 6; ++i_face)
		{
			surfs_.push_back( subresource(i_face, lod_level)->make_mip_surface(filter) );
		}
	}
}

END_NS_SALVIAR();
