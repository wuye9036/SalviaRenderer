#pragma once

#include <salviar/include/salviar_forward.h>
#include <salviar/include/colors.h>
#include <salviar/include/colors_convertors.h>
#include <salviar/include/enums.h>
#include <eflib/include/math/collision_detection.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/function.hpp>
#include <eflib/include/platform/boost_end.h>

#include <vector>

#define SALVIA_TILED_SURFACE 0

BEGIN_NS_SALVIAR();

struct internal_mapped_resource;

class surface
{
public:
	surface();
	surface(size_t width, size_t height, size_t num_samples, pixel_format pxfmt);
	~surface();

	result map(internal_mapped_resource& mapped, map_mode mm);
	result unmap(internal_mapped_resource& mapped, map_mode mm);

	void resolve(surface& target);

	void transfer(pixel_format srcfmt, const eflib::rect<size_t>& dest_rect, void* pdata);
	void transfer(const eflib::rect<size_t>& dest_rect, size_t src_start_x, size_t src_start_y, surface& src_surf);

	size_t get_width() const
    {
		return width_;
	}

	size_t get_height() const
    {
		return height_;
	}

	eflib::int4 get_int_size() const
	{
		return eflib::int4(static_cast<int>(width_), static_cast<int>(height_), 1, 0);
	}

	size_t sample_count() const
    {
		return num_samples_;
	}

	size_t get_pitch() const
    {
		return width_ * num_samples_ * color_infos[pxfmt_].size;
	}

	pixel_format get_pixel_format() const
    {
		return pxfmt_;
	}

    void*         texel_address(size_t x, size_t y, size_t sample);
	void const*   texel_address(size_t x, size_t y, size_t sample) const;

	color_rgba32f get_texel(size_t x, size_t y, size_t sample) const;
    color_rgba32f get_texel(size_t x0, size_t y0, size_t x1, size_t y1, float tx, float ty, size_t sample) const;
	void		  get_texel(void* color, size_t x, size_t y, size_t sample) const;

	void		  set_texel(size_t x, size_t y, size_t sample, const color_rgba32f& color);
	void		  set_texel(size_t x, size_t y, size_t sample, const void* color);

	void		  fill_texels(size_t sx, size_t sy, size_t width, size_t height, const color_rgba32f& color);
    void		  fill_texels(color_rgba32f const& color);

private:
	std::vector<byte, eflib::aligned_allocator<byte, 16>>
					datas_;
	size_t			width_;
	size_t			height_;
	size_t			elem_size_;
	pixel_format	pxfmt_;
	size_t			num_samples_;

#if SALVIA_TILED_SURFACE
	size_t			tile_width_;
	size_t			tile_height_;
	bool			tile_mode_;
#endif

	size_t texel_offset(size_t x, size_t y, size_t sample) const;

#if SALVIA_TILED_SURFACE
	void tile	(internal_mapped_resource const& mapped);
	void untile	(internal_mapped_resource& mapped);
#endif

	pixel_format_convertor::pixel_convertor         to_rgba32_func_;
	pixel_format_convertor::pixel_convertor         from_rgba32_func_;
	pixel_format_convertor::pixel_array_convertor   to_rgba32_array_func_;
	pixel_format_convertor::pixel_array_convertor   from_rgba32_array_func_;
	pixel_format_convertor::pixel_lerp_1d           lerp_1d_func_;
	pixel_format_convertor::pixel_lerp_2d           lerp_2d_func_;
};

END_NS_SALVIAR();
