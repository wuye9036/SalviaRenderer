#include <salviar/include/texture.h>

#include <salviar/include/surface.h>
#include <salviar/include/sampler.h>
#include <salviar/include/internal_mapped_resource.h>

#include <eflib/include/utility/unref_declarator.h>

#include <vector>
#include <memory>

namespace salviar{

using namespace eflib;
using std::vector;
using std::make_shared;

texture_2d::texture_2d(size_t width, size_t height, size_t num_samples, pixel_format format)
{
	fmt_  = format;
	sample_count_ = static_cast<int>(num_samples);
	size_ = uint4(static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1, 0);
	surfs_.push_back( make_shared<surface>(width, height, num_samples, format) );
}

void texture_2d::gen_mipmap(filter_type filter, bool auto_gen)
{
	if(auto_gen)
    {
		max_lod_ = 0;
		min_lod_ = calc_lod_limit(size_) - 1;
	}

	surfs_.reserve(min_lod_ + 1);

	for(size_t lod_level = max_lod_; lod_level < min_lod_; ++lod_level)
	{
		surfs_.push_back( surfs_.back()->make_mip_surface(filter) );
	}
}

}
