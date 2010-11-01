#ifndef SOFTART_SURFACE_H
#define SOFTART_SURFACE_H

#include "colors.h"
#include "colors_convertors.h"
#include "enums.h"

#include "eflib/include/math.h"

#include <vector>
#include "softart_fwd.h"
BEGIN_NS_SOFTART()

//#define TILE_BASED_STORAGE

class surface
{	
private:
	//raw memories
	std::vector<byte> datas_;

	//surface raw format
	size_t width_;
	size_t height_;
	size_t elem_size_;
	pixel_format pxfmt_;
	size_t num_samples_;

#ifdef TILE_BASED_STORAGE
	size_t tile_width_;
	size_t tile_height_;

	bool tile_mode_;
#endif

	//lock information
	bool is_mapped_;
	std::vector<byte> mapped_data_;
	map_mode mm_;

	//lock mode judegement
	bool is_read_mode(map_mode lm){return (map_read == lm) || (map_read_write == lm);}
	bool is_write_mode(map_mode lm){return (map_write == lm) || (map_read_write == lm) || (map_write_discard == lm) || (map_write_no_overwrite == lm);}

	size_t get_texel_addr(size_t x, size_t y, size_t sample) const;

	void tile(const std::vector<byte>& tile_data);
	void untile(std::vector<byte>& untile_data);

	pixel_format_convertor::pixel_convertor to_rgba32_func_;
	pixel_format_convertor::pixel_convertor from_rgba32_func_;
	pixel_format_convertor::pixel_array_convertor to_rgba32_array_func_;
	pixel_format_convertor::pixel_array_convertor from_rgba32_array_func_;
	pixel_format_convertor::pixel_lerp_1d lerp_1d_func_;
	pixel_format_convertor::pixel_lerp_2d lerp_2d_func_;

public:
	surface();
	surface(size_t width, size_t height, size_t num_samples_, pixel_format pxfmt);
	~surface();

	void release();

	void rebuild(size_t width, size_t height, size_t num_samples_, pixel_format pxfmt);

	void map(void** pdata, map_mode mm) const;
	void map(void** pdata, map_mode mm);
	void unmap() const;
	void unmap();

	void resolve(surface& target);

	void transfer(pixel_format srcfmt, const eflib::rect<size_t>& dest_rect, void* pdata);
	void transfer(const eflib::rect<size_t>& dest_rect, size_t src_start_x, size_t src_start_y, surface& src_surf);

	size_t get_width() const{
		return width_;
	}

	size_t get_height() const{
		return height_;
	}

	size_t get_num_samples() const{
		return num_samples_;
	}

	size_t get_pitch() const{
		return width_ * num_samples_ * color_infos[pxfmt_].size;
	}

	pixel_format get_pixel_format() const{
		return pxfmt_;
	}
	bool is_mapped() const{
		return is_mapped_;
	}

	color_rgba32f get_texel(size_t x, size_t y, size_t sample) const;
	void get_texel(void* color, size_t x, size_t y, size_t sample) const;
	color_rgba32f get_texel(size_t x0, size_t y0, size_t x1, size_t y1, float tx, float ty, size_t sample) const;
	void set_texel(size_t x, size_t y, size_t sample, const color_rgba32f& color);
	void set_texel(size_t x, size_t y, size_t sample, const void* color);

	void fill_texels(size_t sx, size_t sy, size_t width, size_t height, const color_rgba32f& color);
};

END_NS_SOFTART()

#endif
