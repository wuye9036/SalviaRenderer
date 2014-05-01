#include <salviar/include/texture.h>

#include <salviar/include/surface.h>
#include <salviar/include/sampler.h>
#include <salviar/include/internal_mapped_resource.h>

#include <eflib/include/utility/unref_declarator.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/make_shared.hpp>
#include <eflib/include/platform/boost_end.h>

#include <vector>

BEGIN_NS_SALVIAR();

using namespace eflib;
using std::vector;
using boost::make_shared;

texture_2d::texture_2d(size_t width, size_t height, size_t num_samples, pixel_format format)
{
	fmt_  = format;
	sample_count_ = num_samples;
	size_ = int4(width, height, 1, 0);
	surfs_.push_back( make_shared<surface>(width, height, num_samples, format) );
}

void texture_2d::gen_mipmap(filter_type filter, bool auto_gen)
{
	if(auto_gen)
    {
		max_lod_ = 0;
		min_lod_ = calc_lod_limit(width_, height_) - 1;
	}

	size_t cur_width    = surfs_[max_lod_]->get_width();
	size_t cur_height   = surfs_[max_lod_]->get_height();
	size_t num_samples  = surfs_[max_lod_]->sample_count();

	surfs_.resize(min_lod_ + 1);

	for(size_t lod_level = max_lod_ + 1; lod_level <= min_lod_; ++lod_level)
	{
		cur_width =  (cur_width + 1) / 2;
		cur_height = (cur_height + 1) / 2;

		surfs_[lod_level] = make_shared<surface>(cur_width, cur_height, num_samples, fmt_);

		switch (filter)
		{
		case filter_point:
			for(size_t y = 0; y < cur_height; ++y)
            {
				for(size_t x = 0; x < cur_width; ++x)
                {
					for(size_t s = 0; s < num_samples; ++s)
					{
						color_rgba32f c = surfs_[lod_level-1]->get_texel(x*2, y*2, s);
						surfs_[lod_level]->set_texel(x, y, s, c);
					}
				}
			}
			break;

		case filter_linear:
			for(size_t y = 0; y < cur_height; ++y)
            {
				for(size_t x = 0; x < cur_width; ++x)
                {
					for(size_t s = 0; s < num_samples; ++s)
					{
						color_rgba32f c[4] =
						{
							surfs_[lod_level-1]->get_texel(x*2+0, y*2+0, s),
							surfs_[lod_level-1]->get_texel(x*2+1, y*2+0, s),
							surfs_[lod_level-1]->get_texel(x*2+0, y*2+1, s),
							surfs_[lod_level-1]->get_texel(x*2+1, y*2+1, s)
						};

						color_rgba32f dst_color(
							(c[0].get_vec4() + c[1].get_vec4() + c[2].get_vec4() + c[3].get_vec4()) * 0.25f
							);
						surfs_[lod_level]->set_texel(x, y, s, dst_color);
					}
				}
			}
			break;
		}
	}
}

size_t texture_2d::sample_count(size_t subresource) const
{
	EFLIB_ASSERT(max_lod_ <= subresource && subresource <= min_lod_, "Mipmap level is out of bound.");

	return get_surface(subresource)->sample_count();
}

void texture_2d::set_max_lod(size_t miplevel)
{
	EFLIB_ASSERT(max_lod_ <= min_lod_, "Max lod is less than min lod.");

	if(! (max_lod_ <= min_lod_)) return;
	max_lod_ = miplevel;
}

void texture_2d::set_min_lod(size_t miplevel)
{
	size_t ml_limit = calc_lod_limit(surfs_[0]->get_width());
	EFLIB_ASSERT(max_lod_ <= miplevel, "Mip level is larger than max LoD level that is set by user.");
	EFLIB_ASSERT(miplevel < ml_limit, "Mip level is larger than max LoD level that texture supported.");

	if(! (max_lod_ <= miplevel && miplevel < ml_limit)) return;
	min_lod_ = miplevel;
}

END_NS_SALVIAR();
