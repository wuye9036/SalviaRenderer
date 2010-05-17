#include "../include/surface.h"

BEGIN_NS_SOFTART()

const size_t TILE_BITS = 5;
const size_t TILE_SIZE = 1UL << TILE_BITS;
const size_t TILE_MASK = TILE_SIZE - 1;

surface::surface():is_mapped_(false)
{
}

surface::surface(size_t width, size_t height, pixel_format pxfmt)
		:pxfmt_(pxfmt), width_(width), height_(height),
		elem_size_(color_infos[pxfmt].size),
		tile_width_((width + TILE_SIZE - 1) >> TILE_BITS),
		tile_height_((height + TILE_SIZE - 1) >> TILE_BITS),
		tile_mode_(true),
		is_mapped_(false)
{
	datas_.resize(tile_width_ * tile_height_ * TILE_SIZE * TILE_SIZE * elem_size_),

	to_rgba32_func_ = pixel_format_convertor::get_convertor_func(pixel_format_color_rgba32f, pxfmt_);
	from_rgba32_func_ = pixel_format_convertor::get_convertor_func(pxfmt_, pixel_format_color_rgba32f);
	to_rgba32_array_func_ = pixel_format_convertor::get_array_convertor_func(pixel_format_color_rgba32f, pxfmt_);
	from_rgba32_array_func_ = pixel_format_convertor::get_array_convertor_func(pxfmt_, pixel_format_color_rgba32f);
	lerp_1d_func_ = pixel_format_convertor::get_lerp_1d_func(pxfmt_);
	lerp_2d_func_ = pixel_format_convertor::get_lerp_2d_func(pxfmt_);
}

surface::~surface()
{
	release();
}

void surface::release(){
	custom_assert(!is_mapped_, "");
	if(is_mapped()){
		std::vector<byte>().swap(mapped_data_);
	}
}

void surface::rebuild(size_t width, size_t height, pixel_format pxfmt)
{
	release();
	new(this) surface(width, height, pxfmt);
}

void surface::map(void** pdata, map_mode mm) const{
	const_cast<surface * const>(this)->map(pdata, mm);
}

void surface::map(void** pdata, map_mode mm){
	custom_assert(!is_mapped(), "不可重复锁定！");

	if(is_mapped()){
		*pdata = NULL;
		return;
	}

	mapped_data_.resize(elem_size_ * width_ * height_);
	*pdata = &mapped_data_[0];

	is_mapped_ = true;
	mm_ = mm;

	if(is_read_mode(mm)){
		this->untile(mapped_data_);
	}
}

void surface::unmap() const{
	const_cast<surface * const>(this)->unmap();
}

void surface::unmap()
{
	custom_assert(is_mapped(), "对未锁定的surface解锁！");

	if(!is_mapped()){
		return;
	}

	if(is_write_mode(mm_)){
		this->tile(mapped_data_);
	}

	std::vector<byte>().swap(mapped_data_);
	is_mapped_ = false;
}

color_rgba32f surface::get_texel(size_t x, size_t y) const
{
	color_rgba32f color;
	to_rgba32_func_(&color, &datas_[this->get_texel_addr(x, y)]);
	return color;
}

color_rgba32f surface::get_texel(size_t x0, size_t y0, size_t x1, size_t y1, float tx, float ty) const
{
	return lerp_2d_func_(&datas_[this->get_texel_addr(x0, y0)], &datas_[this->get_texel_addr(x1, y0)],
		&datas_[this->get_texel_addr(x0, y1)], &datas_[this->get_texel_addr(x1, y1)], tx, ty);
}
	
void surface::set_texel(size_t x, size_t y, const color_rgba32f& color)
{
	from_rgba32_func_(&datas_[get_texel_addr(x, y)], &color);
}

void surface::fill_texels(size_t sx, size_t sy, size_t width, size_t height, const color_rgba32f& color)
{
	uint8_t pix_clr[4 * 4 * sizeof(float)];
	from_rgba32_func_(pix_clr, &color);

	if (tile_mode_){
		if ((0 == sx) && (0 == sy) && (width == width_) && (height == height_)){
			for (uint32_t x = 0; x < TILE_SIZE; ++ x){
				memcpy(&datas_[this->get_texel_addr(x, 0)], pix_clr, elem_size_);
			}
			for (uint32_t y = 1; y < TILE_SIZE; ++ y){
				memcpy(&datas_[this->get_texel_addr(0, y)], &datas_[this->get_texel_addr(0, 0)], TILE_SIZE * elem_size_);
			}
			for (uint32_t tx = 1; tx < tile_width_; ++ tx){
				memcpy(&datas_[this->get_texel_addr(tx << TILE_BITS, 0)], &datas_[this->get_texel_addr(0, 0)], TILE_SIZE * TILE_SIZE * elem_size_);
			}
			for (uint32_t ty = 1; ty < tile_height_; ++ ty){
				memcpy(&datas_[this->get_texel_addr(0, ty << TILE_BITS)], &datas_[this->get_texel_addr(0, 0)], TILE_SIZE * TILE_SIZE * tile_width_ * elem_size_);
			}
		}
		else{
			size_t begin_tile_x = sx >> TILE_BITS;
			size_t begin_x_in_tile = sx & TILE_MASK;
			size_t end_tile_x = (sx + width - 1) >> TILE_BITS;
			size_t end_x_in_tile = (sx + width - 1) & TILE_MASK;

			{
				for (size_t x = begin_x_in_tile; x < TILE_SIZE; ++ x){
					memcpy(&datas_[this->get_texel_addr((begin_tile_x << TILE_BITS) + x, sy)],
						pix_clr, elem_size_);
				}
				for (size_t y = sy + 1; y < sy + height; ++ y){
					memcpy(&datas_[this->get_texel_addr(sx, y)],
						&datas_[this->get_texel_addr(sx, sy)], (TILE_SIZE - begin_x_in_tile) * elem_size_);
				}
			}

			for (size_t tx = begin_tile_x + 1; tx < end_tile_x; ++ tx){
				for (uint32_t x = 0; x < TILE_SIZE; ++ x){
					memcpy(&datas_[this->get_texel_addr((tx << TILE_BITS) + x, sy)],
						pix_clr, elem_size_);
				}
				for (size_t y = sy + 1; y < sy + height; ++ y){
					memcpy(&datas_[this->get_texel_addr(tx << TILE_BITS, y)],
						&datas_[this->get_texel_addr(tx << TILE_BITS, sy)], TILE_SIZE * elem_size_);
				}
			}

			{
				for (size_t x = 0; x < end_x_in_tile; ++ x){
					memcpy(&datas_[this->get_texel_addr((end_tile_x << TILE_BITS) + x, sy)],
						pix_clr, elem_size_);
				}
				for (size_t y = sy + 1; y < sy + height; ++ y){
					memcpy(&datas_[this->get_texel_addr(end_tile_x << TILE_BITS, y)],
						&datas_[this->get_texel_addr(end_tile_x << TILE_BITS, sy)], end_x_in_tile * elem_size_);
				}
			}
		}
	}
	else{
		for (size_t x = sx; x < sx + width; ++ x){
			memcpy(&datas_[(width_ * sy + x) * elem_size_], pix_clr, elem_size_);
		}
		for (size_t y = sy + 1; y < sy + height; ++ y){
			memcpy(&datas_[(width_ * y + sx) * elem_size_], &datas_[(width_ * sy + sx) * elem_size_], elem_size_ * width);
		}
	}
}

size_t surface::get_texel_addr(size_t x, size_t y) const
{
	if (tile_mode_){
		size_t tile_x = x >> TILE_BITS;
		size_t tile_y = y >> TILE_BITS;
		size_t x_in_tile = x & TILE_MASK;
		size_t y_in_tile = y & TILE_MASK;
		return ((tile_y * tile_width_ + tile_x) * TILE_SIZE * TILE_SIZE + (y_in_tile * TILE_SIZE + x_in_tile)) * elem_size_;
	}
	else
	{
		return (y * width_ + x) * elem_size_;
	}
}

void surface::tile(const std::vector<byte>& tile_data)
{
	if (tile_mode_){
		for (size_t y = 0; y < height_; ++ y){
			for (size_t x = 0; x < width_; ++ x){
				size_t tile_x = x >> TILE_BITS;
				size_t tile_y = y >> TILE_BITS;
				size_t x_in_tile = x & TILE_MASK;
				size_t y_in_tile = y & TILE_MASK;
				memcpy(&datas_[((tile_y * tile_width_ + tile_x) * TILE_SIZE * TILE_SIZE + (y_in_tile * TILE_SIZE + x_in_tile)) * elem_size_],
					&tile_data[(y * width_ + x) * elem_size_], elem_size_);
			}
		}
	}
	else{
		datas_ = tile_data;
	}
}

void surface::untile(std::vector<byte>& untile_data)
{
	if (tile_mode_){
		for (size_t ty = 0; ty < tile_height_; ++ ty){
			for (size_t tx = 0; tx < tile_width_; ++ tx){
				for (size_t y = 0; y < std::min(height_ - (ty << TILE_BITS), TILE_SIZE); ++ y){
					memcpy(&untile_data[(((ty << TILE_BITS) + y) * width_ + (tx << TILE_BITS)) * elem_size_],
						&datas_[((ty * tile_width_ + tx) * TILE_SIZE * TILE_SIZE + y * TILE_SIZE) * elem_size_],
						std::min(width_ - (tx << TILE_BITS), TILE_SIZE) * elem_size_);
				}
			}
		}
	}
	else{
		untile_data = datas_;
	}
}

END_NS_SOFTART()
