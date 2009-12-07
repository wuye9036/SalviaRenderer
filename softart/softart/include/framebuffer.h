#ifndef SOFTART_FRAMEBUFFER_H
#define SOFTART_FRAMEBUFFER_H

#include "enums.h"
#include "decl.h"
#include "colors.h"
#include "render_stage.h"
#include "shaderregs.h"

#include <vector>

class framebuffer : public render_stage
{
private:
	std::vector<boost::shared_ptr<surface> > back_cbufs_;
	boost::shared_ptr<surface> dbuf_;
	boost::shared_ptr<surface> sbuf_;
	
	std::vector<bool> buf_valids;
	std::vector<surface* > cbufs_; //framebuffer没有释放surface的权力

	size_t width_, height_;
	pixel_format fmt_;

	backbuffer_pixel_out target_pixel_;

	bool check_buf(surface* psurf);

public:
	//inherited
	void initialize(renderer_impl* pparent);

	framebuffer(size_t width, size_t height, pixel_format fmt);
	~framebuffer(void);

	//重置
	void reset(size_t width, size_t height, pixel_format fmt);

	//渲染目标设置
	void set_render_target_disabled(render_target tar, size_t tar_id);
	void set_render_target_enabled(render_target tar, size_t tar_id);

	void set_render_target(render_target tar, size_t tar_id, surface* psurf);
	surface* get_render_target(render_target tar, size_t tar_id) const;

	//获得渲染状态	
	efl::rect<size_t> get_rect();
	size_t get_width() const;
	size_t get_height() const;
	pixel_format get_buffer_format() const;

	//渲染
	void render_pixel(size_t x, size_t y, const ps_output& ps);

	//清理
	void clear_color(size_t tar_id, const color_rgba32f& c);
	void clear_depth(float d);
	void clear_stencil(int32_t s);
	void clear_color(size_t tar_id, const efl::rect<size_t>& rc, const color_rgba32f& c);
	void clear_depth(const efl::rect<size_t>& rc, float d);
	void clear_stencil(const efl::rect<size_t>& rc, int32_t s);
};

//DECL_HANDLE(framebuffer, h_framebuffer);

#endif