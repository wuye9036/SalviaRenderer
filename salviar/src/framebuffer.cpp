#include <salviar/include/framebuffer.h>
#include <salviar/include/shader.h>
#include <salviar/include/shaderregs.h>
#include <salviar/include/surface.h>
#include <salviar/include/render_state.h>
#include <salviar/include/renderer.h>

#include <eflib/include/math/collision_detection.h>

#include <algorithm>

BEGIN_NS_SALVIAR();

using namespace eflib;
using namespace std;

int32_t mask_stencil_0(int32_t /*stencil*/, int32_t /*mask*/){
	return 0;
}

int32_t mask_stencil_1(int32_t stencil, int32_t mask){
	return stencil & mask;
}

int32_t read_stencil_0(const pixel_accessor& /*target_pixel*/, size_t /*sample*/, int32_t /*mask*/){
	return 0;
}

int32_t read_stencil_1(const pixel_accessor& target_pixel, size_t sample, int32_t mask){
	return target_pixel.stencil(sample) & mask;
}


template <typename T>
bool cmp_func_never(T /*lhs*/, T /*rhs*/){
	return false;
}

template <typename T>
bool cmp_func_less(T lhs, T rhs){
	return lhs < rhs;
}

template <typename T>
bool cmp_func_equal(T lhs, T rhs){
	return lhs == rhs;
}

template <typename T>
bool cmp_func_less_equal(T lhs, T rhs){
	return lhs <= rhs;
}

template <typename T>
bool cmp_func_greater(T lhs, T rhs){
	return lhs > rhs;
}

template <typename T>
bool cmp_func_not_equal(T lhs, T rhs){
	return lhs != rhs;
}

template <typename T>
bool cmp_func_greater_equal(T lhs, T rhs){
	return lhs >= rhs;
}

template <typename T>
bool cmp_func_always(T /*lhs*/, T /*rhs*/){
	return true;
}

int32_t sop_keep(int32_t /*ref*/, int32_t cur_stencil){
	return cur_stencil;
}

int32_t sop_zero(int32_t /*ref*/, int32_t /*cur_stencil*/){
	return 0;
}

int32_t sop_replace(int32_t ref, int32_t /*cur_stencil*/){
	return ref;
}

int32_t sop_incr_sat(int32_t /*ref*/, int32_t cur_stencil){
	return std::min<int32_t>(0xFF, cur_stencil + 1);
}

int32_t sop_decr_sat(int32_t /*ref*/, int32_t cur_stencil){
	return std::max<int32_t>(0, cur_stencil - 1);
}

int32_t sop_invert(int32_t /*ref*/, int32_t cur_stencil){
	return ~cur_stencil;
}

int32_t sop_incr_wrap(int32_t /*ref*/, int32_t cur_stencil){
	return (cur_stencil + 1) & 0xFF;
}

int32_t sop_decr_wrap(int32_t /*ref*/, int32_t cur_stencil){
	return (cur_stencil - 1 + 256) & 0xFF;
}

void write_depth_depth_mask_0(size_t /*sample*/, float /*depth*/, pixel_accessor& /*target_pixel*/){
}

void write_depth_depth_mask_1(size_t sample, float depth, pixel_accessor& target_pixel){
	target_pixel.depth(sample, depth);
}

void write_stencil_0(size_t /*sample*/, int32_t /*stencil*/, int32_t /*mask*/, pixel_accessor& /*target_pixel*/){
}

void write_stencil_1(size_t sample, int32_t stencil, int32_t mask, pixel_accessor& target_pixel){
	target_pixel.stencil(sample, stencil & mask);
}

depth_stencil_state::depth_stencil_state(const depth_stencil_desc& desc)
	: desc_(desc)
{
	if (desc.depth_enable){
		switch (desc.depth_func){
		case compare_function_never:
			depth_test_func_ = cmp_func_never<float>;
			break;
		case compare_function_less:
			depth_test_func_ = cmp_func_less<float>;
			break;
		case compare_function_equal:
			depth_test_func_ = cmp_func_equal<float>;
			break;
		case compare_function_less_equal:
			depth_test_func_ = cmp_func_less_equal<float>;
			break;
		case compare_function_greater:
			depth_test_func_ = cmp_func_greater<float>;
			break;
		case compare_function_not_equal:
			depth_test_func_ = cmp_func_not_equal<float>;
			break;
		case compare_function_greater_equal:
			depth_test_func_ = cmp_func_greater_equal<float>;
			break;
		case compare_function_always:
			depth_test_func_ = cmp_func_always<float>;
			break;
		default:
			EFLIB_ASSERT(false, "");
			break;
		}
	}
	else{
		depth_test_func_ = cmp_func_always<float>;
	}

	if (desc.stencil_enable){
		mask_stencil_func_ = mask_stencil_1;
		read_stencil_func_ = read_stencil_1;

		compare_function stencil_funcs[2] = {
			desc.front_face.stencil_func,
			desc.back_face.stencil_func
		};
		for (int i = 0; i < 2; ++ i){
			switch (stencil_funcs[i]){
			case compare_function_never:
				stencil_test_func_[i] = cmp_func_never<int32_t>;
				break;
			case compare_function_less:
				stencil_test_func_[i] = cmp_func_less<int32_t>;
				break;
			case compare_function_equal:
				stencil_test_func_[i] = cmp_func_equal<int32_t>;
				break;
			case compare_function_less_equal:
				stencil_test_func_[i] = cmp_func_less_equal<int32_t>;
				break;
			case compare_function_greater:
				stencil_test_func_[i] = cmp_func_greater<int32_t>;
				break;
			case compare_function_not_equal:
				stencil_test_func_[i] = cmp_func_not_equal<int32_t>;
				break;
			case compare_function_greater_equal:
				stencil_test_func_[i] = cmp_func_greater_equal<int32_t>;
				break;
			case compare_function_always:
				stencil_test_func_[i] = cmp_func_always<int32_t>;
				break;
			default:
				EFLIB_ASSERT(false, "");
				break;
			}
		}

		stencil_op sops[6] = {
			desc.front_face.stencil_fail_op,
			desc.front_face.stencil_pass_op,
			desc.front_face.stencil_depth_fail_op,
			desc.back_face.stencil_fail_op,
			desc.back_face.stencil_pass_op,
			desc.back_face.stencil_depth_fail_op
		};
		for (int i = 0; i < 6; ++ i){
			switch (sops[i]){
			case stencil_op_keep:
				stencil_op_func_[i] = sop_keep;
				break;
			case stencil_op_zero:
				stencil_op_func_[i] = sop_zero;
				break;
			case stencil_op_replace:
				stencil_op_func_[i] = sop_replace;
				break;
			case stencil_op_incr_sat:
				stencil_op_func_[i] = sop_incr_sat;
				break;
			case stencil_op_decr_sat:
				stencil_op_func_[i] = sop_decr_sat;
				break;
			case stencil_op_invert:
				stencil_op_func_[i] = sop_invert;
				break;
			case stencil_op_incr_wrap:
				stencil_op_func_[i] = sop_incr_wrap;
				break;
			case stencil_op_decr_wrap:
				stencil_op_func_[i] = sop_decr_wrap;
				break;
			default:
				EFLIB_ASSERT(false, "");
				break;
			}
		}
	}
	else{
		mask_stencil_func_ = mask_stencil_0;
		read_stencil_func_ = read_stencil_0;

		for (int i = 0; i < 2; ++ i){
			stencil_test_func_[i] = cmp_func_always<int32_t>;
		}

		for (int i = 0; i < 6; ++ i){
			stencil_op_func_[i] = sop_keep;
		}
	}

	if (desc.depth_write_mask){
		write_depth_func_ = write_depth_depth_mask_1;
	}
	else{
		write_depth_func_ = write_depth_depth_mask_0;
	}
	if (desc.stencil_enable){
		write_stencil_func_ = write_stencil_1;
	}
	else{
		write_stencil_func_ = write_stencil_0;
	}
}

const depth_stencil_desc& depth_stencil_state::get_desc() const{
	return desc_;
}

int32_t depth_stencil_state::read_stencil(int32_t stencil) const{
	return mask_stencil_func_(stencil, desc_.stencil_read_mask);
}

int32_t depth_stencil_state::read_stencil(const pixel_accessor& target_pixel, size_t sample) const{
	return read_stencil_func_(target_pixel, sample, desc_.stencil_read_mask);
}

bool depth_stencil_state::depth_test(float ps_depth, float cur_depth) const{
	return depth_test_func_(ps_depth, cur_depth);
}

bool depth_stencil_state::stencil_test(bool front_face, int32_t ref, int32_t cur_stencil) const{
	return stencil_test_func_[!front_face](ref, cur_stencil);
}

int32_t depth_stencil_state::stencil_operation(bool front_face, bool depth_pass, bool stencil_pass, int32_t ref, int32_t cur_stencil) const{
	return stencil_op_func_[(!front_face) * 3 + (!depth_pass) + static_cast<int>(stencil_pass)](ref, cur_stencil);
}

void depth_stencil_state::write_depth(size_t sample, float depth, pixel_accessor& target_pixel) const{
	write_depth_func_(sample, depth, target_pixel);
}

void depth_stencil_state::write_stencil(size_t sample, int32_t stencil, pixel_accessor& target_pixel) const{
	write_stencil_func_(sample, stencil, desc_.stencil_write_mask, target_pixel);
}

/****************************************************
 *   Framebuffer
 ***************************************************/
bool framebuffer::check_buf(surface* psurf){
	return psurf && (psurf->get_width() < width_ || psurf->get_height() < height_);
}

void framebuffer::initialize(render_stages* /*stages*/)
{
}

void framebuffer::update(render_state* state)
{
	ds_state_	= state->ds_state.get();
	stencil_ref_= ds_state_->read_stencil(state->stencil_ref);
}

framebuffer::framebuffer(renderer_parameters const* params)
{
	reset(
		params->backbuffer_width, params->backbuffer_height,
		params->backbuffer_num_samples, params->backbuffer_format
		);
}

framebuffer::framebuffer(size_t width, size_t height, size_t num_samples, pixel_format fmt)
{
	reset(width, height, num_samples, fmt);
}

framebuffer::~framebuffer()
{
}

/// The first render target is framebuffer surface and others are null.
void framebuffer::reset(size_t width, size_t height, size_t num_samples, pixel_format fmt)
{
	width_			= width;
	height_			= height;
	num_samples_	= num_samples;
	fmt_			= fmt;
	back_cbufs_		= std::vector<surface_ptr>(pso_color_regcnt);
	cbufs_			= std::vector<surface*>(pso_color_regcnt);
	buf_valids		= std::vector<bool>(pso_color_regcnt);
	dbuf_			.reset(new surface(width, height, num_samples, pixel_format_color_r32f));
	sbuf_			.reset(new surface(width, height, num_samples, pixel_format_color_r32i));

	for(auto& buf: back_cbufs_)
	{
		buf.reset();
	}

	back_cbufs_[0].reset(new surface(width, height, num_samples, fmt));

	for(size_t i = 0; i < cbufs_.size(); ++i)
	{
		cbufs_[i] = back_cbufs_[i].get();
	}

	buf_valids[0] = true;
	for(size_t i = 1; i < buf_valids.size(); ++i)
	{
		buf_valids[i] = false;
	}
}

// Redirect render target. Just support color buffer for now.
void framebuffer::set_render_target(render_target tar, size_t target_index, surface* psurf)
{
	EFLIB_ASSERT_AND_IF(tar == render_target_color, "You cannot set any type of render target except color type."){
		return;
	}
	EFLIB_ASSERT_AND_IF(target_index < cbufs_.size(), "Index of target is out of bound"){
		return;
	}

	// If surface passed into function is null, then activate back buffer as current.
	if(!psurf){
		cbufs_[target_index] = back_cbufs_[target_index].get();
		return;
	}

	EFLIB_ASSERT(psurf->get_width() >= width_ && psurf->get_height() >= height_, "Size of render target is not enough.");
	cbufs_[target_index] = psurf;
}

surface* framebuffer::get_render_target(render_target tar, size_t target_index) const
{
	EFLIB_ASSERT_AND_IF(tar == render_target_color, "You cannot get any render target except color."){
		return NULL;
	}

	EFLIB_ASSERT_AND_IF(target_index < cbufs_.size(), "Index of target is out of bound"){
		return NULL;
	}

	return cbufs_[target_index];
}
	
rect<size_t> framebuffer::get_rect()
{
	return rect<size_t>(0, 0, width_, height_);
}

size_t framebuffer::get_width() const{
	return width_;
}

size_t framebuffer::get_height() const{
	return height_;
}

size_t framebuffer::get_num_samples() const{
	return num_samples_;
}

pixel_format framebuffer::get_buffer_format() const{
	return fmt_;
}

void framebuffer::render_sample(cpp_blend_shader* cpp_bs, size_t x, size_t y, size_t i_sample, const ps_output& ps, float depth)
{
	EFLIB_ASSERT(cpp_bs, "Blend shader is null or invalid.");
	if(!cpp_bs) return;

	//composing output
	pixel_accessor target_pixel(cbufs_, dbuf_.get(), sbuf_.get());
	target_pixel.set_pos(x, y);

	float   const cur_depth   = target_pixel.depth(i_sample);
	int32_t const cur_stencil = ds_state_->read_stencil(target_pixel, i_sample);

	bool depth_passed	= ds_state_->depth_test(depth, cur_depth);
	bool stencil_passed = ds_state_->stencil_test(ps.front_face, stencil_ref_, cur_stencil);

	if (depth_passed && stencil_passed)
	{
		int32_t new_stencil = ds_state_->stencil_operation(ps.front_face, depth_passed, stencil_passed, stencil_ref_, cur_stencil);

		//execute target shader
		cpp_bs->execute(i_sample, target_pixel, ps);

		ds_state_->write_depth(i_sample, depth, target_pixel);
		ds_state_->write_stencil(i_sample, new_stencil, target_pixel);
	}
}

float framebuffer::get_z(size_t x, size_t y) const
{
	return dbuf_->get_texel(x, y, 0).r;
}

void framebuffer::clear_color(size_t target_index, const color_rgba32f& c){

	EFLIB_ASSERT(target_index < cbufs_.size(), "Target index is out of bound.");
	EFLIB_ASSERT(cbufs_[target_index] && buf_valids[target_index], "Cannot clear an invalid target.");

	cbufs_[target_index]->fill_texels(0, 0, width_, height_, c);
}

void framebuffer::clear_depth(float d){
	dbuf_->fill_texels(0, 0, width_, height_, color_rgba32f(d, 0, 0, 0));
}

void framebuffer::clear_stencil(int32_t s){
	sbuf_->fill_texels(0, 0, width_, height_, color_rgba32f(float(s), 0, 0, 0));
}

void framebuffer::clear_color(size_t target_index, const rect<size_t>& rc, const color_rgba32f& c){

	EFLIB_ASSERT(target_index < cbufs_.size(), "Target index is out of bound");
	EFLIB_ASSERT(cbufs_[target_index] && buf_valids[target_index], "Cannot clear an invalid target.");
	EFLIB_ASSERT(rc.w + rc.x <= width_ && rc.h +rc.y <= height_, "Invalid clear region.");

	cbufs_[target_index]->fill_texels(rc.x, rc.y, rc.w, rc.h, c);
}

void framebuffer::clear_depth(const rect<size_t>& rc, float d){
	EFLIB_ASSERT(rc.w + rc.x <= width_ && rc.h +rc.y <= height_, "Invalid clear region.");

	dbuf_->fill_texels(rc.x, rc.y, rc.w, rc.h, color_rgba32f(d, 0, 0, 0));
}

void framebuffer::clear_stencil(const rect<size_t>& rc, int32_t s){
	EFLIB_ASSERT(rc.w + rc.x <= width_ && rc.h +rc.y <= height_, "Invalid clear region.");

	sbuf_->fill_texels(rc.x, rc.y, rc.w, rc.h, color_rgba32f(float(s), 0, 0, 0));
}

END_NS_SALVIAR();
