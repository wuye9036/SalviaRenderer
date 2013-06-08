#pragma once

#include <salviar/include/salviar_forward.h>

#include <salviar/include/enums.h>
#include <salviar/include/decl.h>
#include <salviar/include/colors.h>
#include <salviar/include/render_stage.h>

#include <eflib/include/math/collision_detection.h>

#include <vector>

BEGIN_NS_SALVIAR();

struct pixel_accessor;

struct depth_stencil_op_desc {
    stencil_op stencil_fail_op;
    stencil_op stencil_depth_fail_op;
    stencil_op stencil_pass_op;
    compare_function stencil_func;

	depth_stencil_op_desc()
		: stencil_fail_op(stencil_op_keep),
			stencil_depth_fail_op(stencil_op_keep),
			stencil_pass_op(stencil_op_keep),
			stencil_func(compare_function_always){
	}
};

struct depth_stencil_desc {
    bool depth_enable;
    bool depth_write_mask;
    compare_function depth_func;
    bool stencil_enable;
    uint8_t stencil_read_mask;
    uint8_t stencil_write_mask;
    depth_stencil_op_desc front_face;
    depth_stencil_op_desc back_face;

	depth_stencil_desc()
		: depth_enable(true),
			depth_write_mask(true),
			depth_func(compare_function_less),
			stencil_enable(false),
			stencil_read_mask(0xFF), stencil_write_mask(0xFF){
	}
};

class depth_stencil_state {
	depth_stencil_desc desc_;

	typedef int32_t (*mask_stencil_func_type)(int32_t stencil, int32_t mask);
	typedef int32_t (*read_stencil_func_type)(const pixel_accessor& target_pixel, size_t sample, int32_t mask);
	typedef bool (*depth_test_func_type)(float ps_depth, float cur_depth);
	typedef bool (*stencil_test_func_type)(int32_t ref, int32_t cur_stencil);
	typedef int32_t (*stencil_op_func_type)(int32_t ref, int32_t cur_stencil);
	typedef void (*write_depth_func_type)(size_t sample, float depth, pixel_accessor& target_pixel);
	typedef void (*write_stencil_func_type)(size_t sample, int32_t stencil, int32_t mask, pixel_accessor& target_pixel);

	mask_stencil_func_type mask_stencil_func_;
	read_stencil_func_type read_stencil_func_;
	depth_test_func_type depth_test_func_;
	stencil_test_func_type stencil_test_func_[2];
	stencil_op_func_type stencil_op_func_[6];
	write_depth_func_type write_depth_func_;
	write_stencil_func_type write_stencil_func_;

public:
	depth_stencil_state(const depth_stencil_desc& desc);
	const depth_stencil_desc& get_desc() const;

	int32_t read_stencil(int32_t stencil) const;
	int32_t read_stencil(const pixel_accessor& target_pixel, size_t sample) const;
	bool depth_test(float ps_depth, float cur_depth) const;
	bool stencil_test(bool front_face, int32_t ref, int32_t cur_stencil) const;
	int32_t stencil_operation(bool front_face, bool depth_pass, bool stencil_pass, int32_t ref, int32_t cur_stencil) const;
	void write_depth(size_t sample, float depth, pixel_accessor& target_pixel) const;
	void write_stencil(size_t sample, int32_t stencil, pixel_accessor& target_pixel) const;
};

class framebuffer : public render_stage
{
private:
	std::vector<boost::shared_ptr<surface> > back_cbufs_;
	boost::shared_ptr<surface> dbuf_;
	boost::shared_ptr<surface> sbuf_;
	
	std::vector<bool> buf_valids;
	std::vector<surface* > cbufs_; ///< NOTE: Framebuffer doesn't take ownership of surfaces

	size_t width_, height_;
	size_t num_samples_;
	pixel_format fmt_;

	bool check_buf(surface* psurf);

public:
	//inherited
	void initialize(renderer_impl* pparent);

	framebuffer(size_t width, size_t height, size_t num_samples, pixel_format fmt);
	~framebuffer(void);

	void reset(size_t width, size_t height, size_t num_samples, pixel_format fmt);

	// Render target accessors.
	void set_render_target_disabled(render_target tar, size_t target_index);
	void set_render_target_enabled(render_target tar, size_t target_index);

	void set_render_target(render_target tar, size_t target_index, surface* psurf);
	surface* get_render_target(render_target tar, size_t target_index) const;

	// Render states accessors.
	eflib::rect<size_t> get_rect();
	size_t get_width() const;
	size_t get_height() const;
	size_t get_num_samples() const;
	pixel_format get_buffer_format() const;

	float get_z(size_t x, size_t y) const;

	// Rendering functions.
	void render_sample(cpp_blend_shader* cpp_bs, size_t x, size_t y, size_t i_sample, const ps_output& ps, float depth);

	// Cleanup functions.
	void clear_color(size_t target_index, const color_rgba32f& c);
	void clear_depth(float d);
	void clear_stencil(int32_t s);
	void clear_color(size_t target_index, const eflib::rect<size_t>& rc, const color_rgba32f& c);
	void clear_depth(const eflib::rect<size_t>& rc, float d);
	void clear_stencil(const eflib::rect<size_t>& rc, int32_t s);
};

END_NS_SALVIAR();