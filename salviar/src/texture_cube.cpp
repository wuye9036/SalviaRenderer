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

END_NS_SALVIAR();
