#include "../include/surface.h"

BEGIN_NS_SOFTART()

#ifdef TILE_BASED_STORAGE
const size_t TILE_BITS = 5;
const size_t TILE_SIZE = 1UL << TILE_BITS;
const size_t TILE_MASK = TILE_SIZE - 1;
#endif

surface::surface():is_mapped_(false)
{
}

surface::surface(size_t width, size_t height, size_t num_samples, pixel_format pxfmt)
		:pxfmt_(pxfmt), width_(width), height_(height), num_samples_(num_samples),
		elem_size_(color_infos[pxfmt].size),
		is_mapped_(false)
{
#ifdef TILE_BASED_STORAGE
	tile_width_ = (width + TILE_SIZE - 1) >> TILE_BITS;
	tile_height_ = (height + TILE_SIZE - 1) >> TILE_BITS;

	tile_mode_ = true;
	if ((TILE_SIZE > width_) || (TILE_SIZE > height_)){
		tile_mode_ = false;
	}

	if (tile_mode_){
		datas_.resize(tile_width_ * tile_height_ * TILE_SIZE * TILE_SIZE * num_samples_ * elem_size_);
	}
	else{
		datas_.resize(width_ * height_ * num_samples_ * elem_size_);
	}
#else
	datas_.resize(width_ * height_ * num_samples_ * elem_size_);
#endif

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

void surface::rebuild(size_t width, size_t height, size_t num_samples, pixel_format pxfmt)
{
	release();
	new(this) surface(width, height, num_samples, pxfmt);
}

void surface::map(void** pdata, map_mode mm) const{
	const_cast<surface * const>(this)->map(pdata, mm);
}

void surface::map(void** pdata, map_mode mm){
	custom_assert(!is_mapped(), "不可重复锁定！");
	custom_assert(1 == num_samples_, "Can't map a multi-sample surface!");

	if (num_samples_ > 1){
		*pdata = NULL;
		return;
	}

	is_mapped_ = true;
	mm_ = mm;

#ifdef TILE_BASED_STORAGE
	mapped_data_.resize(elem_size_ * width_ * height_);
	*pdata = &mapped_data_[0];

	if(is_read_mode(mm)){
		this->untile(mapped_data_);
	}
#else
	if (map_read == mm){
		*pdata = &datas_[0];
	}
	else{
		mapped_data_.resize(elem_size_ * width_ * height_);
		*pdata = &mapped_data_[0];

		if(is_read_mode(mm)){
			this->untile(mapped_data_);
		}
	}
#endif
}

void surface::unmap() const{
	const_cast<surface * const>(this)->unmap();
}

void surface::unmap()
{
	custom_assert(is_mapped(), "对未锁定的surface解锁！");

	if(is_write_mode(mm_)){
		this->tile(mapped_data_);
	}

	mapped_data_.resize(0);
	is_mapped_ = false;
}

void surface::resolve(surface& target)
{
	custom_assert(1 == target.get_num_samples(), "Resolve's target can't be a multi-sample surface");

	color_rgba32f clr;
	color_rgba32f tmp;
	for (size_t y = 0; y < height_; ++ y){
		for (size_t x = 0; x < width_; ++ x){
			clr = color_rgba32f(0, 0, 0, 0);
			for (size_t s = 0; s < num_samples_; ++ s){
				to_rgba32_func_(&tmp, &datas_[this->get_texel_addr(x, y, s)]);
				clr.get_vec4() += tmp.get_vec4();
			}
			clr.get_vec4() /= static_cast<float>(num_samples_);

			target.set_texel(x, y, 0, clr);
		}
	}
}

color_rgba32f surface::get_texel(size_t x, size_t y, size_t sample) const
{
#ifndef EFLIB_NO_SIMD
	_mm_prefetch(reinterpret_cast<const char*>(&datas_[0]) + this->get_texel_addr(x, y, 0) + 64, _MM_HINT_NTA);
#endif

	color_rgba32f color;
	to_rgba32_func_(&color, &datas_[this->get_texel_addr(x, y, sample)]);
	return color;
}

void surface::get_texel(void* color, size_t x, size_t y, size_t sample) const
{
#ifndef EFLIB_NO_SIMD
	_mm_prefetch(reinterpret_cast<const char*>(&datas_[0]) + this->get_texel_addr(x, y, 0) + 64, _MM_HINT_NTA);
#endif

	memcpy(color, &datas_[this->get_texel_addr(x, y, sample)], elem_size_);
}

color_rgba32f surface::get_texel(size_t x0, size_t y0, size_t x1, size_t y1, float tx, float ty, size_t sample) const
{
#ifndef EFLIB_NO_SIMD
	_mm_prefetch(reinterpret_cast<const char*>(&datas_[0]) + this->get_texel_addr(x0, y0, 0) + 64, _MM_HINT_NTA);
#endif

	return lerp_2d_func_(&datas_[this->get_texel_addr(x0, y0, sample)], &datas_[this->get_texel_addr(x1, y0, sample)],
		&datas_[this->get_texel_addr(x0, y1, sample)], &datas_[this->get_texel_addr(x1, y1, sample)], tx, ty);
}
	
void surface::set_texel(size_t x, size_t y, size_t sample, const color_rgba32f& color)
{
	from_rgba32_func_(&datas_[get_texel_addr(x, y, sample)], &color);
}

void surface::set_texel(size_t x, size_t y, size_t sample, const void* color)
{
	memcpy(&datas_[get_texel_addr(x, y, sample)], color, elem_size_);
}

void surface::fill_texels(size_t sx, size_t sy, size_t width, size_t height, const color_rgba32f& color)
{
	uint8_t pix_clr[4 * 4 * sizeof(float)];
	from_rgba32_func_(pix_clr, &color);

#ifdef TILE_BASED_STORAGE
	if (tile_mode_){
		if ((0 == sx) && (0 == sy) && (width == width_) && (height == height_)){
			for (uint32_t x = 0; x < TILE_SIZE; ++ x){
				for (size_t s = 0; s < num_samples_; ++ s){
					memcpy(&datas_[this->get_texel_addr(x, 0, s)], pix_clr, elem_size_);
				}
			}
			for (uint32_t y = 1; y < TILE_SIZE; ++ y){
				memcpy(&datas_[this->get_texel_addr(0, y)], &datas_[this->get_texel_addr(0, 0)], TILE_SIZE * num_samples_ * elem_size_);
			}
			for (uint32_t tx = 1; tx < tile_width_; ++ tx){
				memcpy(&datas_[this->get_texel_addr(tx << TILE_BITS, 0)], &datas_[this->get_texel_addr(0, 0)], TILE_SIZE * TILE_SIZE * num_samples_ * elem_size_);
			}
			for (uint32_t ty = 1; ty < tile_height_; ++ ty){
				memcpy(&datas_[this->get_texel_addr(0, ty << TILE_BITS)], &datas_[this->get_texel_addr(0, 0)], TILE_SIZE * TILE_SIZE * tile_width_ * num_samples_ * elem_size_);
			}
		}
		else{
			size_t begin_tile_x = sx >> TILE_BITS;
			size_t begin_x_in_tile = sx & TILE_MASK;
			size_t end_tile_x = (sx + width - 1) >> TILE_BITS;
			size_t end_x_in_tile = (sx + width - 1) & TILE_MASK;

			{
				for (size_t x = begin_x_in_tile; x < TILE_SIZE; ++ x){
					for (size_t s = 0; s < num_samples_; ++ s){
						memcpy(&datas_[this->get_texel_addr((begin_tile_x << TILE_BITS) + x, sy, s)],
							pix_clr, elem_size_);
					}
				}
				for (size_t y = sy + 1; y < sy + height; ++ y){
					memcpy(&datas_[this->get_texel_addr(sx, y)],
						&datas_[this->get_texel_addr(sx, sy)], (TILE_SIZE - begin_x_in_tile) * num_samples_ * elem_size_);
				}
			}

			for (size_t tx = begin_tile_x + 1; tx < end_tile_x; ++ tx){
				for (uint32_t x = 0; x < TILE_SIZE; ++ x){
					for (size_t s = 0; s < num_samples_; ++ s){
						memcpy(&datas_[this->get_texel_addr((tx << TILE_BITS) + x, sy, s)],
							pix_clr, elem_size_);
					}
				}
				for (size_t y = sy + 1; y < sy + height; ++ y){
					memcpy(&datas_[this->get_texel_addr(tx << TILE_BITS, y)],
						&datas_[this->get_texel_addr(tx << TILE_BITS, sy)], TILE_SIZE * num_samples_ * elem_size_);
				}
			}

			{
				for (size_t x = 0; x < end_x_in_tile; ++ x){
					for (size_t s = 0; s < num_samples_; ++ s){
						memcpy(&datas_[this->get_texel_addr((end_tile_x << TILE_BITS) + x, sy, s)],
							pix_clr, elem_size_);
					}
				}
				for (size_t y = sy + 1; y < sy + height; ++ y){
					memcpy(&datas_[this->get_texel_addr(end_tile_x << TILE_BITS, y)],
						&datas_[this->get_texel_addr(end_tile_x << TILE_BITS, sy)], end_x_in_tile * num_samples_ * elem_size_);
				}
			}
		}
	}
	else{
		for (size_t x = sx; x < sx + width; ++ x){
			for (size_t s = 0; s < num_samples_; ++ s){
				memcpy(&datas_[((width_ * sy + x) * num_samples_ + s) * elem_size_], pix_clr, elem_size_);
			}
		}
		for (size_t y = sy + 1; y < sy + height; ++ y){
			memcpy(&datas_[(width_ * y + sx) * num_samples_ * elem_size_], &datas_[(width_ * sy + sx) * num_samples_ * elem_size_], num_samples_ * elem_size_ * width);
		}
	}
#else
	for (size_t x = sx; x < sx + width; ++ x){
		for (size_t s = 0; s < num_samples_; ++ s){
			memcpy(&datas_[((width_ * sy + x) * num_samples_ + s) * elem_size_], pix_clr, elem_size_);
		}
	}
	for (size_t y = sy + 1; y < sy + height; ++ y){
		memcpy(&datas_[(width_ * y + sx) * num_samples_ * elem_size_], &datas_[(width_ * sy + sx) * num_samples_ * elem_size_], num_samples_ * elem_size_ * width);
	}
#endif
}

size_t surface::get_texel_addr(size_t x, size_t y, size_t sample) const
{
#ifdef TILE_BASED_STORAGE
	if (tile_mode_){
		const size_t tile_x = x >> TILE_BITS;
		const size_t tile_y = y >> TILE_BITS;
		const size_t x_in_tile = x & TILE_MASK;
		const size_t y_in_tile = y & TILE_MASK;
		return (((tile_y * tile_width_ + tile_x) * TILE_SIZE * TILE_SIZE + (y_in_tile * TILE_SIZE + x_in_tile)) * num_samples_ + sample) * elem_size_;
	}
	else
	{
		return ((y * width_ + x) * num_samples_ + sample) * elem_size_;
	}
#else
	return ((y * width_ + x) * num_samples_ + sample) * elem_size_;
#endif
}

void surface::tile(const std::vector<byte>& tile_data)
{
#ifdef TILE_BASED_STORAGE
	if (tile_mode_){
		for (size_t ty = 0; ty < tile_height_; ++ ty){
			const size_t by = ty << TILE_BITS;
			const size_t rest_height = std::min(height_ - by, TILE_SIZE);
			for (size_t tx = 0; tx < tile_width_; ++ tx){
				const size_t bx = tx << TILE_BITS;
				const size_t tile_id = ty * tile_width_ + tx;
				const size_t rest_width = std::min(width_ - bx, TILE_SIZE);
				for (size_t y = 0; y < rest_height; ++ y){
					memcpy(&datas_[((tile_id * TILE_SIZE + y) * TILE_SIZE) * num_samples_ * elem_size_],
						&tile_data[((by + y) * width_ + bx) * num_samples_ * elem_size_],
						rest_width * num_samples_ * elem_size_);
				}
			}
		}
	}
	else{
		datas_ = tile_data;
	}
#else
	datas_ = tile_data;
#endif
}

void surface::untile(std::vector<byte>& untile_data)
{
#ifdef TILE_BASED_STORAGE
	if (tile_mode_){
		for (size_t ty = 0; ty < tile_height_; ++ ty){
			const size_t by = ty << TILE_BITS;
			const size_t rest_height = std::min(height_ - by, TILE_SIZE);
			for (size_t tx = 0; tx < tile_width_; ++ tx){
				const size_t bx = tx << TILE_BITS;
				const size_t tile_id = ty * tile_width_ + tx;
				const size_t rest_width = std::min(width_ - bx, TILE_SIZE);
				for (size_t y = 0; y < rest_height; ++ y){
					memcpy(&untile_data[((by + y) * width_ + bx) * num_samples_ * elem_size_],
						&datas_[((tile_id * TILE_SIZE + y) * TILE_SIZE) * num_samples_ * elem_size_],
						rest_width * num_samples_ * elem_size_);
				}
			}
		}
	}
	else{
		untile_data = datas_;
	}
#else
	untile_data = datas_;
#endif
}

END_NS_SOFTART()
