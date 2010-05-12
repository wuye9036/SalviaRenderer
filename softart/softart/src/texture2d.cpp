#include "eflib/include/util.h"

#include "../include/texture.h"

#include "../include/surface.h"
#include "../include/sampler.h"
BEGIN_NS_SOFTART()

using namespace efl;

texture_2d::texture_2d(size_t width, size_t height, pixel_format format):is_locked_(false), locked_surface_(0)
{
	width_ = width;
	height_ = height;

	fmt_ = format;
	surfs_.resize(1);
	surfs_[0].rebuild(width, height, format);
}

void texture_2d::reset(size_t width, size_t height, pixel_format format)
{
	custom_assert(!is_locked_, "重置纹理时发现纹理已被锁定！");

	new(this) texture_2d(width, height, format);
}

void  texture_2d::gen_mipmap(filter_type filter)
{
	size_t cur_sizex = surfs_[max_lod_].get_width();
	size_t cur_sizey = surfs_[max_lod_].get_height();

	surfs_.resize(min_lod_ + 1);

	int elem_size = color_infos[fmt_].size;
	for(size_t iLevel = max_lod_ + 1; iLevel <= min_lod_; ++iLevel)
	{
		size_t last_sizex = cur_sizex;
		size_t last_sizey = cur_sizey;

		cur_sizex = (cur_sizex + 1) / 2;
		cur_sizey = (cur_sizey + 1) / 2;

		byte* dst_data = NULL;
		byte* src_data = NULL;

		surfs_[iLevel].rebuild(cur_sizex, cur_sizey, fmt_);
		surfs_[iLevel].lock((void**)&dst_data, rect<size_t>(0, 0, cur_sizex, cur_sizey), lock_write_only);
		surfs_[iLevel - 1].lock((void**)&src_data, rect<size_t>(0, 0, last_sizex, last_sizey), lock_read_only);

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

					color_rgba32f c = (c0.get_vec4() + c1.get_vec4() + c2.get_vec4() + c3.get_vec4()) * 0.25f;

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

		surfs_[iLevel - 1].unlock();
		surfs_[iLevel].unlock();
	}
}

void  texture_2d::lock(void** pData, size_t miplevel, const rect<size_t>& lrc, lock_mode lm, size_t z_slice)
{
	custom_assert(max_lod_ <= miplevel && miplevel <= min_lod_, "Mipmap Level越界！");
	custom_assert(z_slice == 0, "z_slice选项设定无效。");
	custom_assert(pData != 0, "pData不可为NULL！");
	custom_assert(!is_locked_, "试图重复锁定已锁定的纹理！");
	UNREF_PARAM(z_slice);

	*pData = NULL;

	if(is_locked_) return;
	locked_surface_ = miplevel;
	surfs_[locked_surface_].lock(pData, lrc, lm);
	if(*pData == NULL){return;}

	is_locked_ = true;
}

void  texture_2d::unlock()
{
	custom_assert(is_locked_, "试图对未锁定的纹理解锁！");
	if(! is_locked_) return;
	surfs_[locked_surface_].unlock();
	is_locked_ = false;
}

surface&  texture_2d::get_surface(size_t miplevel, size_t z_slice)
{
	custom_assert(max_lod_ <= miplevel && miplevel <= min_lod_, "Mipmap Level越界！");
	custom_assert(z_slice == 0, "z_slice选项设定无效。");
	UNREF_PARAM(z_slice);

	return surfs_[miplevel];
}

const surface&  texture_2d::get_surface(size_t miplevel, size_t z_slice) const
{
	custom_assert(max_lod_ <= miplevel && miplevel <= min_lod_, "Mipmap Level越界！");
	custom_assert(z_slice == 0, "z_slice选项设定无效。");
	UNREF_PARAM(z_slice);

	return surfs_[miplevel];
}

size_t  texture_2d::get_width(size_t mipmap) const
{
	custom_assert(max_lod_ <= mipmap && mipmap <= min_lod_, "Mipmap Level越界！");

	if(mipmap == 0) return width_;
	return surfs_[mipmap].get_width();
}

size_t  texture_2d::get_height(size_t mipmap) const
{
	custom_assert(max_lod_ <= mipmap && mipmap <= min_lod_, "Mipmap Level越界！");

	if(mipmap == 0) return height_;
	return surfs_[mipmap].get_width();
}

size_t  texture_2d::get_depth(size_t mipmap) const
{
	custom_assert(max_lod_ <= mipmap && mipmap <= min_lod_, "Mipmap Level越界！");
	UNREF_PARAM(mipmap);

	return 1;
}

void  texture_2d::set_max_lod(size_t miplevel)
{
	custom_assert(max_lod_ <= min_lod_, "最低细节的Mip等级设置错误！");

	if(! (max_lod_ <= min_lod_)) return;
	max_lod_ = miplevel;
}

void  texture_2d::set_min_lod(size_t miplevel)
{
	size_t ml_limit = calc_lod_limit(surfs_[0].get_width());
	custom_assert(max_lod_ <= miplevel && miplevel < ml_limit, "最低细节的Mip等级设置错误！");

	if(! (max_lod_ <= miplevel && miplevel < ml_limit)) return;
	min_lod_ = miplevel;
}
END_NS_SOFTART()
