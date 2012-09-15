#include <salviar/include/texture.h>

#include <salviar/include/surface.h>
#include <salviar/include/sampler.h>

#include <eflib/include/utility/unref_declarator.h>

BEGIN_NS_SALVIAR()

using namespace eflib;

texture_2d::texture_2d(size_t width, size_t height, size_t num_samples, pixel_format format)
{
	width_ = width;
	height_ = height;

	fmt_ = format;
	surfs_.resize(1);
	surfs_[0].rebuild(width, height, num_samples, format);
}

void texture_2d::reset(size_t width, size_t height, size_t num_samples, pixel_format format)
{
	new(this) texture_2d(width, height, num_samples, format);
}

void texture_2d::gen_mipmap(filter_type filter, bool auto_gen)
{
	if( auto_gen ){
		max_lod_ = 0;
		min_lod_ = calc_lod_limit( width_, height_ ) - 1;
	}

	size_t cur_sizex = surfs_[max_lod_].get_width();
	size_t cur_sizey = surfs_[max_lod_].get_height();
	size_t num_samples = surfs_[max_lod_].get_num_samples();

	surfs_.resize(min_lod_ + 1);

	int elem_size = color_infos[fmt_].size;
	for(size_t iLevel = max_lod_ + 1; iLevel <= min_lod_; ++iLevel)
	{
		size_t last_sizex = cur_sizex;

		cur_sizex = (cur_sizex + 1) / 2;
		cur_sizey = (cur_sizey + 1) / 2;

		byte* dst_data = NULL;
		byte* src_data = NULL;

		surfs_[iLevel].rebuild(cur_sizex, cur_sizey, num_samples, fmt_);
		surfs_[iLevel].map((void**)&dst_data, map_write);
		surfs_[iLevel - 1].map((void**)&src_data, map_read);

#if 0
		float r = iLevel % 3 == 0 ? 1.0f : 0.0f;
		float g = iLevel % 3 == 1 ? 1.0f : 0.0f;
		float b = iLevel % 3 == 2 ? 1.0f : 0.0f;
		color_rgba32f c(r, g, b, 1.0f);
#endif

		switch (filter)
		{
		case filter_point:
			for(size_t iPixely = 0; iPixely < cur_sizey; ++iPixely){
				for(size_t iPixelx = 0; iPixelx < cur_sizex; ++iPixelx){
					color_rgba32f c;
					pixel_format_convertor::convert(
						pixel_format_color_rgba32f, fmt_, 
						(void*)&c, (const void*)(src_data + ((iPixelx * 2) + (iPixely * 2) * last_sizex) * elem_size)
						);

					pixel_format_convertor::convert(
						fmt_, pixel_format_color_rgba32f, 
						(void*)(dst_data + (iPixelx + iPixely * cur_sizex) * elem_size), (const void*)&c
						);
				}
			}
			break;

		case filter_linear:
			for(size_t iPixely = 0; iPixely < cur_sizey; ++iPixely){
				for(size_t iPixelx = 0; iPixelx < cur_sizex; ++iPixelx){
					color_rgba32f c0, c1, c2, c3;
					pixel_format_convertor::convert(
						pixel_format_color_rgba32f, fmt_, 
						(void*)&c0, (const void*)(src_data + ((iPixelx * 2 + 0) + (iPixely * 2 + 0) * last_sizex) * elem_size)
						);
					pixel_format_convertor::convert(
						pixel_format_color_rgba32f, fmt_, 
						(void*)&c1, (const void*)(src_data + ((iPixelx * 2 + 1) + (iPixely * 2 + 0) * last_sizex) * elem_size)
						);
					pixel_format_convertor::convert(
						pixel_format_color_rgba32f, fmt_, 
						(void*)&c2, (const void*)(src_data + ((iPixelx * 2 + 0) + (iPixely * 2 + 1) * last_sizex) * elem_size)
						);
					pixel_format_convertor::convert(
						pixel_format_color_rgba32f, fmt_, 
						(void*)&c3, (const void*)(src_data + ((iPixelx * 2 + 1) + (iPixely * 2 + 1) * last_sizex) * elem_size)
						);

					color_rgba32f c((c0.get_vec4() + c1.get_vec4() + c2.get_vec4() + c3.get_vec4()) * 0.25f);

					pixel_format_convertor::convert(
						fmt_, pixel_format_color_rgba32f, 
						(void*)(dst_data + (iPixelx + iPixely * cur_sizex) * elem_size), (const void*)&c
						);
				}
			}
			break;

		default:
			break;
		}

		surfs_[iLevel - 1].unmap();
		surfs_[iLevel].unmap();
	}
}

void  texture_2d::map(void** pdata, size_t subresource, map_mode mm)
{
	EFLIB_ASSERT(max_lod_ <= subresource && subresource <= min_lod_, "Mipmap level is out of bound.");
	assert(pdata);

#ifdef EFLIB_MSVC
#pragma warning(push)
#pragma warning(disable : 6011)
#endif
	*pdata = NULL;
#ifdef EFLIB_MSVC
#pragma warning(pop)
#endif
	get_surface(subresource).map(pdata, mm);
}

void  texture_2d::unmap(size_t subresource)
{
	get_surface(subresource).unmap();
}

surface&  texture_2d::get_surface(size_t subresource)
{
	EFLIB_ASSERT(max_lod_ <= subresource && subresource <= min_lod_, "Mipmap level is out of bound.");

	return surfs_[subresource];
}

const surface& texture_2d::get_surface(size_t subresource) const
{
	EFLIB_ASSERT(max_lod_ <= subresource && subresource <= min_lod_, "Mipmap level is out of bound.");

	return surfs_[subresource];
}

size_t texture_2d::get_width(size_t subresource) const
{
	EFLIB_ASSERT(max_lod_ <= subresource && subresource <= min_lod_, "Mipmap level is out of bound.");

	return get_surface(subresource).get_width();
}

size_t texture_2d::get_height(size_t subresource) const
{
	EFLIB_ASSERT(max_lod_ <= subresource && subresource <= min_lod_, "Mipmap level is out of bound.");

	return get_surface(subresource).get_width();
}

size_t texture_2d::get_depth(size_t subresource) const
{
	EFLIB_ASSERT(max_lod_ <= subresource && subresource <= min_lod_, "Mipmap level is out of bound.");
	EFLIB_UNREF_DECLARATOR(subresource);

	return 1;
}

size_t texture_2d::get_num_samples(size_t subresource) const
{
	EFLIB_ASSERT(max_lod_ <= subresource && subresource <= min_lod_, "Mipmap level is out of bound.");

	return get_surface(subresource).get_num_samples();
}

void texture_2d::set_max_lod(size_t miplevel)
{
	EFLIB_ASSERT(max_lod_ <= min_lod_, "Max lod is less than min lod.");

	if(! (max_lod_ <= min_lod_)) return;
	max_lod_ = miplevel;
}

void texture_2d::set_min_lod(size_t miplevel)
{
	size_t ml_limit = calc_lod_limit(surfs_[0].get_width());
	EFLIB_ASSERT(max_lod_ <= miplevel, "Mip level is larger than max LoD level that is set by user.");
	EFLIB_ASSERT(miplevel < ml_limit, "Mip level is larger than max LoD level that texture supported.");

	if(! (max_lod_ <= miplevel && miplevel < ml_limit)) return;
	min_lod_ = miplevel;
}
END_NS_SALVIAR()
