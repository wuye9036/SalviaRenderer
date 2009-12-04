#ifndef SOFTART_SURFACE_H
#define SOFTART_SURFACE_H

#include "colors.h"
#include "enums.h"

#include "eflib/include/math.h"

#include <vector>

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

	//lock information
	bool is_locked_;
	efl::rect<size_t> locked_rect_;
	void* plockeddata_;
	lock_mode lm_;

	//lock mode judegement
	bool is_read_mode(lock_mode lm){return ((int)lm & (int)lock_read_only) != 0;}
	bool is_write_mode(lock_mode lm){return ((int)lm & (int)lock_write_only) != 0;}

public:
	surface():is_locked_(false){
	}

	surface(size_t width, size_t height, pixel_format pxfmt)
		:datas_(width*height*color_infos[pxfmt].size),
		pxfmt_(pxfmt), width_(width),	height_(height),
		elem_size_(color_infos[pxfmt].size),is_locked_(false)
	{}

	~surface(){
		release();
	}

	void release(){
		custom_assert(!is_locked_, "");
		if(is_locked()) delete [] plockeddata_;
	}

	void rebuild(size_t width, size_t height, pixel_format pxfmt)
	{
		release();
		new(this) surface(width, height, pxfmt);
	}

	void lock(void** pdata, const efl::rect<size_t>& rc, lock_mode lm){
		custom_assert(rc.h > 0 && rc.w > 0, "锁定区域大小需为正数！");
		custom_assert(rc.x + rc.w <= get_width() && rc.y + rc.h <= get_height(), "锁定区域超界！");
		custom_assert(rc.x >= 0 && rc.y >= 0, "锁定起始点不可为负.");
		custom_assert(! is_locked(), "不可重复锁定！");

		if(is_locked()){	
			*pdata = NULL;
			return;
		}

		plockeddata_ = *pdata = new byte[elem_size_ * rc.w * rc.h];
		if(*pdata == NULL){return;}

		is_locked_ = true;
		lm_ = lm;
		locked_rect_ = rc;

		if(is_read_mode(lm)){
			byte* psrc = &datas_[0] + (locked_rect_.y * width_ + locked_rect_.x) * elem_size_;
			byte* pdest = (byte*)(plockeddata_);

			for(size_t irow = 0; irow < locked_rect_.h; ++irow){
				memcpy(pdest, psrc, elem_size_ * locked_rect_.w);
				psrc += width_ * elem_size_;
				pdest += locked_rect_.w * elem_size_;
			}
		}

		return;
	}

	void unlock()
	{
		custom_assert(is_locked(), "对未锁定的surface解锁！");

		if(!is_locked()){
			return;
		}

		if(plockeddata_ && is_write_mode(lm_)){

			byte* pdest = &datas_[0] + (locked_rect_.y * width_ + locked_rect_.x) * elem_size_;
			byte* psrc = (byte*)(plockeddata_);

			for(size_t irow = 0; irow < locked_rect_.h; ++irow){
				memcpy(pdest, psrc, elem_size_ * locked_rect_.w);
				pdest += width_ * elem_size_;
				psrc += locked_rect_.w * elem_size_;
			}
		}

		delete [] plockeddata_;
		plockeddata_ = NULL;
		is_locked_ = false;

		return;
	}

	void transfer(pixel_format srcfmt, const efl::rect<size_t>& dest_rect, void* pdata);
	void transfer(const efl::rect<size_t>& dest_rect, size_t src_start_x, size_t src_start_y, surface& src_surf);

	size_t get_width() const{
		return width_;
	}

	size_t get_height() const{
		return height_;
	}

	size_t get_pitch() const{
		return width_ * color_infos[pxfmt_].size;
	}

	pixel_format get_pixel_format() const{
		return pxfmt_;
	}
	bool is_locked() const{
		return is_locked_;
	}

	color_rgba32f get_texel(size_t x, size_t y) const{
		color_rgba32f color;
		pixel_format_convertor::convert(pixel_format_color_rgba32f, pxfmt_, (void*)(&color), (const void*)&datas_[(width_*y+x)*elem_size_]);
		return color;
	}
	void set_texel(size_t x, size_t y, const color_rgba32f& color){
		pixel_format_convertor::convert(pxfmt_, pixel_format_color_rgba32f, (void*)&datas_[(width_*y+x)*elem_size_], (const void*)(&color));
	}

	void fill_clr_texels(size_t sx, size_t sy, size_t width, size_t height, const color_rgba32f& color){
		pixel_format_convertor::convert(pxfmt_, pixel_format_color_rgba32f, (void*)&datas_[(width_*sy+sx)*elem_size_], (const void*)(&color));
		for (size_t x = sx + 1; x < sx + width; ++ x){
			memcpy(&datas_[(width_ * sy + x) * elem_size_], &datas_[(width_ * sy + sx) * elem_size_], elem_size_);
		}
		for (size_t y = sy + 1; y < sy + height; ++ y){
			memcpy(&datas_[(width_ * y + sx) * elem_size_], &datas_[(width_ * sy + sx) * elem_size_], elem_size_ * width);
		}
	}
};

#endif
