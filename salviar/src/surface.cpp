#include <salviar/include/surface.h>
#include <salviar/include/internal_mapped_resource.h>

#include <memory.h>

using eflib::int4;

BEGIN_NS_SALVIAR();

#if SALVIA_TILED_SURFACE
const size_t TILE_BITS = 5;
const size_t TILE_SIZE = 1UL << TILE_BITS;
const size_t TILE_MASK = TILE_SIZE - 1;
#endif

surface::surface(size_t w, size_t h, size_t samp_count, pixel_format fmt)
		: format_(fmt)
		, size_(static_cast<int>(w), static_cast<int>(h), 1, 0),
		, sample_count_(samp_count), elem_size_(color_infos[fmt].size)
{
#if SALVIA_TILED_SURFACE
	tile_size_[0] = (width + TILE_SIZE - 1) >> TILE_BITS;
	tile_size_[1] = (height + TILE_SIZE - 1) >> TILE_BITS;

	tile_mode_ = true;
	if ((TILE_SIZE > size_[0]) || (TILE_SIZE > size_[1])){
		tile_mode_ = false;
	}

	if (tile_mode_){
		datas_.resize(tile_size_[0] * tile_size_[1] * TILE_SIZE * TILE_SIZE * sample_count_ * elem_size_);
	}
	else{
		datas_.resize(size_[0] * size_[1] * sample_count_ * elem_size_);
	}
#else
	datas_.resize( pitch() * h );
#endif

	to_rgba32_func_         = pixel_format_convertor::get_convertor_func(pixel_format_color_rgba32f, format_);
	from_rgba32_func_       = pixel_format_convertor::get_convertor_func(format_, pixel_format_color_rgba32f);
	to_rgba32_array_func_   = pixel_format_convertor::get_array_convertor_func(pixel_format_color_rgba32f, format_);
	from_rgba32_array_func_ = pixel_format_convertor::get_array_convertor_func(format_, pixel_format_color_rgba32f);
	lerp_1d_func_           = pixel_format_convertor::get_lerp_1d_func(format_);
	lerp_2d_func_           = pixel_format_convertor::get_lerp_2d_func(format_);
}

surface::~surface()
{
}

result surface::map(internal_mapped_resource& mapped, map_mode mm)
{
#if SALVIA_TILED_SURFACE
	// Unimplemented
	this->untile(mapped_data_);
#else
	switch(mm)
	{
	case map_read:
		mapped.data = mapped.reallocator(datas_.size());
		memcpy( mapped.data, datas_.data(), datas_.size() );
		break;
	case map_read_write:
	case map_write_no_overwrite:
	case map_write_discard:
	case map_write:
		mapped.data = datas_.data();
		break;
	}

	mapped.row_pitch = static_cast<uint32_t>( get_pitch() );
	mapped.depth_pitch = static_cast<uint32_t>(mapped.row_pitch * size_[1]);

	return result::ok;
#endif
}

result surface::unmap(internal_mapped_resource& mapped, map_mode mm)
{
	// No intermediate buffer needed in linear mode.
	return result::ok;
}

void surface::resolve(surface& target)
{
	EFLIB_ASSERT(1 == target.sample_count(), "Resolve's target can't be a multi-sample surface");

	color_rgba32f clr;
	color_rgba32f tmp;
	for (size_t y = 0; y < size_[1]; ++ y)
	{
		for (size_t x = 0; x < size_[0]; ++ x)
		{
			clr = color_rgba32f(0, 0, 0, 0);
			for (size_t s = 0; s < sample_count_; ++ s)
			{
				to_rgba32_func_( &tmp, texel_address(x, y, s) );
				clr.get_vec4() += tmp.get_vec4();
			}
			clr.get_vec4() /= static_cast<float>(sample_count_);

			target.set_texel(x, y, 0, clr);
		}
	}
}

color_rgba32f surface::get_texel(size_t x, size_t y, size_t sample) const
{
	color_rgba32f color;
	to_rgba32_func_(&color, texel_address(x, y, sample));
	return color;
}

void surface::get_texel(void* color, size_t x, size_t y, size_t sample) const
{
	memcpy(color, texel_address(x, y, sample), elem_size_);
}

color_rgba32f surface::get_texel(size_t x0, size_t y0, size_t x1, size_t y1, float tx, float ty, size_t sample) const
{
	void const* addrs[] =
	{
		texel_address(x0, y0, sample),
		texel_address(x1, y0, sample),
		texel_address(x0, y1, sample),
		texel_address(x1, y1, sample)
	};

	return lerp_2d_func_(addrs[0], addrs[1], addrs[2], addrs[3], tx, ty);
}

void surface::set_texel(size_t x, size_t y, size_t sample, const color_rgba32f& color)
{
	from_rgba32_func_(texel_address(x, y, sample), &color);
}

void surface::set_texel(size_t x, size_t y, size_t sample, const void* color)
{
	memcpy(texel_address(x, y, sample), color, elem_size_);
}

void surface::fill_texels(size_t sx, size_t sy, size_t width, size_t height, const color_rgba32f& color)
{
	uint8_t pix_clr[4 * 4 * sizeof(float)];
	from_rgba32_func_(pix_clr, &color);

#if SALVIA_TILED_SURFACE
	if (tile_mode_)
    {
		if ((0 == sx) && (0 == sy) && (width == size_[0]) && (height == size_[1]))
        {
			for (uint32_t x = 0; x < TILE_SIZE; ++ x){
				for (size_t s = 0; s < sample_count_; ++ s){
					memcpy(&datas_[this->texel_offset(x, 0, s)], pix_clr, elem_size_);
				}
			}
			for (uint32_t y = 1; y < TILE_SIZE; ++ y){
				memcpy(&datas_[this->texel_offset(0, y, 0)], &datas_[this->texel_offset(0, 0, 0)], TILE_SIZE * sample_count_ * elem_size_);
			}
			for (uint32_t tx = 1; tx < tile_size_[0]; ++ tx){
				memcpy(&datas_[this->texel_offset(tx << TILE_BITS, 0, 0)], &datas_[this->texel_offset(0, 0, 0)], TILE_SIZE * TILE_SIZE * sample_count_ * elem_size_);
			}
			for (uint32_t ty = 1; ty < tile_size_[1]; ++ ty){
				memcpy(&datas_[this->texel_offset(0, ty << TILE_BITS, 0)], &datas_[this->texel_offset(0, 0, 0)], TILE_SIZE * TILE_SIZE * tile_size_[0] * sample_count_ * elem_size_);
			}
		}
		else
        {
			size_t begin_tile_x = sx >> TILE_BITS;
			size_t begin_x_in_tile = sx & TILE_MASK;
			size_t end_tile_x = (sx + width - 1) >> TILE_BITS;
			size_t end_x_in_tile = (sx + width - 1) & TILE_MASK;

			{
				for (size_t x = begin_x_in_tile; x < TILE_SIZE; ++ x){
					for (size_t s = 0; s < sample_count_; ++ s){
						memcpy(&datas_[this->texel_offset((begin_tile_x << TILE_BITS) + x, sy, s)],
							pix_clr, elem_size_);
					}
				}
				for (size_t y = sy + 1; y < sy + height; ++ y){
					memcpy(&datas_[this->texel_offset(sx, y, 0)],
						&datas_[this->texel_offset(sx, sy, 0)], (TILE_SIZE - begin_x_in_tile) * sample_count_ * elem_size_);
				}
			}

			for (size_t tx = begin_tile_x + 1; tx < end_tile_x; ++ tx){
				for (uint32_t x = 0; x < TILE_SIZE; ++ x){
					for (size_t s = 0; s < sample_count_; ++ s){
						memcpy(&datas_[this->texel_offset((tx << TILE_BITS) + x, sy, s)],
							pix_clr, elem_size_);
					}
				}
				for (size_t y = sy + 1; y < sy + height; ++ y){
					memcpy(&datas_[this->texel_offset(tx << TILE_BITS, y, 0)],
						&datas_[this->texel_offset(tx << TILE_BITS, sy, 0)], TILE_SIZE * sample_count_ * elem_size_);
				}
			}

			{
				for (size_t x = 0; x < end_x_in_tile; ++ x){
					for (size_t s = 0; s < sample_count_; ++ s){
						memcpy(&datas_[this->texel_offset((end_tile_x << TILE_BITS) + x, sy, s)],
							pix_clr, elem_size_);
					}
				}
				for (size_t y = sy + 1; y < sy + height; ++ y){
					memcpy(&datas_[this->texel_offset(end_tile_x << TILE_BITS, y, 0)],
						&datas_[this->texel_offset(end_tile_x << TILE_BITS, sy, 0)], end_x_in_tile * sample_count_ * elem_size_);
				}
			}
		}
	}
	else
    {
		for (size_t x = sx; x < sx + width; ++ x){
			for (size_t s = 0; s < sample_count_; ++ s){
				memcpy(&datas_[((size_[0] * sy + x) * sample_count_ + s) * elem_size_], pix_clr, elem_size_);
			}
		}
		for (size_t y = sy + 1; y < sy + height; ++ y){
			memcpy(&datas_[(size_[0] * y + sx) * sample_count_ * elem_size_], &datas_[(size_[0] * sy + sx) * sample_count_ * elem_size_], sample_count_ * elem_size_ * width);
		}
	}
#else
	for (size_t x = sx; x < sx + width; ++ x)
	{
		for (size_t s = 0; s < sample_count_; ++ s)
		{
			memcpy(&datas_[((size_[0] * sy + x) * sample_count_ + s) * elem_size_], pix_clr, elem_size_);
		}
	}

	for (size_t y = sy + 1; y < sy + height; ++ y)
	{
		memcpy(&datas_[(size_[0] * y + sx) * sample_count_ * elem_size_], &datas_[(size_[0] * sy + sx) * sample_count_ * elem_size_], sample_count_ * elem_size_ * width);
	}
#endif
}

void surface::fill_texels(color_rgba32f const& color)
{
    fill_texels(0, 0, size_[0], size_[1], color);
}

size_t surface::texel_offset(size_t x, size_t y, size_t sample) const
{
#if SALVIA_TILED_SURFACE
	if (tile_mode_)
	{
		size_t const tile_x		= x >> TILE_BITS;
		size_t const tile_y		= y >> TILE_BITS;
		size_t const x_in_tile	= x & TILE_MASK;
		size_t const y_in_tile	= y & TILE_MASK;
		return (((tile_y * tile_size_[0] + tile_x) * TILE_SIZE * TILE_SIZE + (y_in_tile * TILE_SIZE + x_in_tile)) * sample_count_ + sample) * elem_size_;
	}
	else
	{
		return ((y * size_[0] + x) * sample_count_ + sample) * elem_size_;
	}
#else
	return ((y * size_[0] + x) * sample_count_ + sample) * elem_size_;
#endif
}

#if SALVIA_TILED_SURFACE
void surface::tile(internal_mapped_resource const& mapped)
{
	if (tile_mode_)
	{
		for (size_t ty = 0; ty < tile_size_[1]; ++ ty)
		{
			const size_t by = ty << TILE_BITS;
			const size_t rest_height = std::min(size_[1] - by, TILE_SIZE);
			for (size_t tx = 0; tx < tile_size_[0]; ++ tx)
			{
				const size_t bx = tx << TILE_BITS;
				const size_t tile_id = ty * tile_size_[0] + tx;
				const size_t rest_width = std::min(size_[0] - bx, TILE_SIZE);
				for (size_t y = 0; y < rest_height; ++ y)
				{
					memcpy(&datas_[((tile_id * TILE_SIZE + y) * TILE_SIZE) * sample_count_ * elem_size_],
						&tile_data[((by + y) * size_[0] + bx) * sample_count_ * elem_size_],
						rest_width * sample_count_ * elem_size_);
				}
			}
		}
	}
	else
	{
		datas_ = tile_data;
	}
}

void surface::untile(std::vector<byte>& untile_data)
{
	if (tile_mode_)
	{
		for (size_t ty = 0; ty < tile_size_[1]; ++ ty)
		{
			const size_t by = ty << TILE_BITS;
			const size_t rest_height = std::min(size_[1] - by, TILE_SIZE);
			for (size_t tx = 0; tx < tile_size_[0]; ++ tx)
			{
				const size_t bx = tx << TILE_BITS;
				const size_t tile_id = ty * tile_size_[0] + tx;
				const size_t rest_width = std::min(size_[0] - bx, TILE_SIZE);
				for (size_t y = 0; y < rest_height; ++ y)
				{
					memcpy(&untile_data[((by + y) * size_[0] + bx) * sample_count_ * elem_size_],
						&datas_[((tile_id * TILE_SIZE + y) * TILE_SIZE) * sample_count_ * elem_size_],
						rest_width * sample_count_ * elem_size_);
				}
			}
		}
	}
	else
	{
		untile_data = datas_;
	}
}

#endif

void* surface::texel_address(size_t x, size_t y, size_t sample)
{
    return reinterpret_cast<void*>( datas_.data() + texel_offset(x, y, sample) );
}

void const* surface::texel_address(size_t x, size_t y, size_t sample) const
{
    return reinterpret_cast<void const*>( datas_.data() + texel_offset(x, y, sample) );
}
END_NS_SALVIAR();
