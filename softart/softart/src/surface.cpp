#include "../include/surface.h"

BEGIN_NS_SOFTART()

surface::surface():is_locked_(false)
{
}

surface::surface(size_t width, size_t height, pixel_format pxfmt)
		:datas_(width*height*color_infos[pxfmt].size),
		pxfmt_(pxfmt), width_(width),	height_(height),
		elem_size_(color_infos[pxfmt].size),is_locked_(false)
{
}

surface::~surface()
{
	release();
}

void surface::release(){
	custom_assert(!is_locked_, "");
	if(is_locked()){
		std::vector<byte>().swap(locked_data_);
	}
}

void surface::rebuild(size_t width, size_t height, pixel_format pxfmt)
{
	release();
	new(this) surface(width, height, pxfmt);
}

void surface::lock_readonly(void** pdata, const efl::rect<size_t>& rc) const{
	const_cast<surface * const>(this)->lock(pdata, rc, lock_read_only);
}

void surface::lock(void** pdata, const efl::rect<size_t>& rc, lock_mode lm){
	custom_assert(rc.h > 0 && rc.w > 0, "锁定区域大小需为正数！");
	custom_assert(rc.x + rc.w <= get_width() && rc.y + rc.h <= get_height(), "锁定区域超界！");
	custom_assert(rc.x >= 0 && rc.y >= 0, "锁定起始点不可为负.");
	custom_assert(! is_locked(), "不可重复锁定！");

	if(is_locked()){	
		*pdata = NULL;
		return;
	}

	locked_data_.resize(elem_size_ * rc.w * rc.h);
	*pdata = &locked_data_[0];

	is_locked_ = true;
	lm_ = lm;
	locked_rect_ = rc;

	if(is_read_mode(lm)){
		byte* psrc = &datas_[0] + (locked_rect_.y * width_ + locked_rect_.x) * elem_size_;
		byte* pdest = &locked_data_[0];

		for(size_t irow = 0; irow < locked_rect_.h; ++irow){
			memcpy(pdest, psrc, elem_size_ * locked_rect_.w);
			psrc += width_ * elem_size_;
			pdest += locked_rect_.w * elem_size_;
		}
	}

	return;
}

void surface::unlock() const{
	const_cast<surface * const>(this)->unlock();
}

void surface::unlock()
{
	custom_assert(is_locked(), "对未锁定的surface解锁！");

	if(!is_locked()){
		return;
	}

	if(is_write_mode(lm_)){

		byte* pdest = &datas_[0] + (locked_rect_.y * width_ + locked_rect_.x) * elem_size_;
		byte* psrc = &locked_data_[0];

		for(size_t irow = 0; irow < locked_rect_.h; ++irow){
			memcpy(pdest, psrc, elem_size_ * locked_rect_.w);
			pdest += width_ * elem_size_;
			psrc += locked_rect_.w * elem_size_;
		}
	}

	std::vector<byte>().swap(locked_data_);
	is_locked_ = false;
}

color_rgba32f surface::get_texel(size_t x, size_t y) const
{
	color_rgba32f color;
	pixel_format_convertor::convert(pixel_format_color_rgba32f, pxfmt_, (void*)(&color), (const void*)&datas_[(width_*y+x)*elem_size_]);
	return color;
}

void surface::set_texel(size_t x, size_t y, const color_rgba32f& color)
{
	pixel_format_convertor::convert(pxfmt_, pixel_format_color_rgba32f, (void*)&datas_[(width_*y+x)*elem_size_], (const void*)(&color));
}

void surface::fill_texels(size_t sx, size_t sy, size_t width, size_t height, const color_rgba32f& color)
{
	set_texel(sx, sy, color);
	for (size_t x = sx + 1; x < sx + width; ++ x){
		memcpy(&datas_[(width_ * sy + x) * elem_size_], &datas_[(width_ * sy + sx) * elem_size_], elem_size_);
	}
	for (size_t y = sy + 1; y < sy + height; ++ y){
		memcpy(&datas_[(width_ * y + sx) * elem_size_], &datas_[(width_ * sy + sx) * elem_size_], elem_size_ * width);
	}
}

END_NS_SOFTART()
