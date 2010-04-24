#ifndef SOFTART_SURFACE_H
#define SOFTART_SURFACE_H

#include "colors.h"
#include "enums.h"

#include "eflib/include/math.h"

#include <vector>
#include "softart_fwd.h"
BEGIN_NS_SOFTART()


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
	std::vector<byte> locked_data_;
	lock_mode lm_;

	//lock mode judegement
	bool is_read_mode(lock_mode lm){return ((int)lm & (int)lock_read_only) != 0;}
	bool is_write_mode(lock_mode lm){return ((int)lm & (int)lock_write_only) != 0;}

public:
	surface();
	surface(size_t width, size_t height, pixel_format pxfmt);
	~surface();

	void release();

	void rebuild(size_t width, size_t height, pixel_format pxfmt);

	void lock_readonly(void** pdata, const efl::rect<size_t>& rc) const;
	void lock(void** pdata, const efl::rect<size_t>& rc, lock_mode lm);
	void unlock() const;
	void unlock();

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

	color_rgba32f get_texel(size_t x, size_t y) const;
	void set_texel(size_t x, size_t y, const color_rgba32f& color);

	void fill_texels(size_t sx, size_t sy, size_t width, size_t height, const color_rgba32f& color);
};

END_NS_SOFTART()

#endif
