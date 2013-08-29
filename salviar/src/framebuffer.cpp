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

template <uint32_t Format>
class depth_stencil_accessor
{
public:
	static float 	read_depth(void const* /*ds_data*/)
    {
    }
	
    static uint32_t	read_stencil(void const* /*ds_data*/)
    {
    }

	static void 	read_depth_stencil(float& /*depth*/, uint32_t& /*stencil*/, void const* /*ds_data*/)
    {
    }

	static void 	write_depth(void* /*ds_data*/, float /*depth*/)
    {
    }

	static void		write_stencil(void* /*ds_data*/, uint32_t /*stencil*/)
    {
    }

	static void 	write_depth_stencil(void* /*ds_data*/, float /*depth*/, uint32_t /*stencil*/)
    {
    }
};

template <> class depth_stencil_accessor<pixel_format_color_rg32f>
{
public:
	static float 	read_depth(void const* ds_data)
    {
        return reinterpret_cast<color_rg32f const*>(ds_data)->r;
    }

	static uint32_t	read_stencil(void const* ds_data)
    {
        union
        {
            float stencil_f;
            uint32_t stencil_u;
        };
        stencil_f = reinterpret_cast<color_rg32f const*>(ds_data)->g;
        return stencil_u;
    }

	static void 	read_depth_stencil(float& depth, uint32_t& stencil, void const* ds_data)
    {
        depth = reinterpret_cast<color_rg32f const*>(ds_data)->r;
        union
        {
            float stencil_f;
            uint32_t stencil_u;
        };
        stencil_f = reinterpret_cast<color_rg32f const*>(ds_data)->g;
        stencil = stencil_u;
    }
	
	static void 	write_depth(void* ds_data, float depth)
    {
        reinterpret_cast<color_rg32f*>(ds_data)->r = depth;
    }

	static void		write_stencil(void* ds_data, uint32_t stencil)
    {
        union
        {
            float stencil_f;
            uint32_t stencil_u;
        };
        stencil_u = stencil;
        reinterpret_cast<color_rg32f*>(ds_data)->g = stencil_f;
    }

	static void 	write_depth_stencil(void* ds_data, float depth, uint32_t stencil)
    {
        reinterpret_cast<color_rg32f*>(ds_data)->r = depth;
        union
        {
            float stencil_f;
            uint32_t stencil_u;
        };
        stencil_u = stencil;
        reinterpret_cast<color_rg32f*>(ds_data)->g = stencil_f;
    }
};

uint32_t mask_stencil_0(uint32_t /*stencil*/, uint32_t /*mask*/)
{
	return 0;
}

uint32_t mask_stencil_1(uint32_t stencil, uint32_t mask)
{
	return stencil & mask;
}

template <typename T>
bool compare_never(T /*lhs*/, T /*rhs*/)
{
	return false;
}

template <typename T>
bool compare_less(T lhs, T rhs){
	return lhs < rhs;
}

template <typename T>
bool compare_equal(T lhs, T rhs){
	return lhs == rhs;
}

template <typename T>
bool compare_less_equal(T lhs, T rhs){
	return lhs <= rhs;
}

template <typename T>
bool compare_greater(T lhs, T rhs){
	return lhs > rhs;
}

template <typename T>
bool compare_not_equal(T lhs, T rhs){
	return lhs != rhs;
}

template <typename T>
bool compare_greater_equal(T lhs, T rhs){
	return lhs >= rhs;
}

template <typename T>
bool compare_always(T /*lhs*/, T /*rhs*/){
	return true;
}

uint32_t sop_keep(uint32_t /*ref*/, uint32_t cur_stencil){
	return cur_stencil;
}

uint32_t sop_zero(uint32_t /*ref*/, uint32_t /*cur_stencil*/){
	return 0;
}

uint32_t sop_replace(uint32_t ref, uint32_t /*cur_stencil*/){
	return ref;
}

uint32_t sop_incr_sat(uint32_t /*ref*/, uint32_t cur_stencil){
	return std::min<uint32_t>(0xFF, cur_stencil + 1);
}

uint32_t sop_decr_sat(uint32_t /*ref*/, uint32_t cur_stencil){
	return std::max<uint32_t>(0, cur_stencil - 1);
}

uint32_t sop_invert(uint32_t /*ref*/, uint32_t cur_stencil){
	return ~cur_stencil;
}

uint32_t sop_incr_wrap(uint32_t /*ref*/, uint32_t cur_stencil){
	return (cur_stencil + 1) & 0xFF;
}

uint32_t sop_decr_wrap(uint32_t /*ref*/, uint32_t cur_stencil){
	return (cur_stencil - 1 + 256) & 0xFF;
}

depth_stencil_state::depth_stencil_state(const depth_stencil_desc& desc)
	: desc_(desc)
{
	if (desc.depth_enable)
    {
		switch (desc.depth_func)
        {
		case compare_function_never:
			depth_test_ = compare_never<float>;
			break;
		case compare_function_less:
			depth_test_ = compare_less<float>;
			break;
		case compare_function_equal:
			depth_test_ = compare_equal<float>;
			break;
		case compare_function_less_equal:
			depth_test_ = compare_less_equal<float>;
			break;
		case compare_function_greater:
			depth_test_ = compare_greater<float>;
			break;
		case compare_function_not_equal:
			depth_test_ = compare_not_equal<float>;
			break;
		case compare_function_greater_equal:
			depth_test_ = compare_greater_equal<float>;
			break;
		case compare_function_always:
			depth_test_ = compare_always<float>;
			break;
		default:
			EFLIB_ASSERT(false, "");
			break;
		}
	}
	else
    {
		depth_test_ = compare_always<float>;
	}

	if (desc.stencil_enable)
    {
		mask_stencil_ = mask_stencil_1;

		compare_function stencil_compare_functions[2] = 
        {
			desc.front_face.stencil_func,
			desc.back_face.stencil_func
		};

		for (int i = 0; i < 2; ++ i)
        {
			switch (stencil_compare_functions[i])
            {
			case compare_function_never:
				stencil_test_[i] = compare_never<uint32_t>;
				break;
			case compare_function_less:
				stencil_test_[i] = compare_less<uint32_t>;
				break;
			case compare_function_equal:
				stencil_test_[i] = compare_equal<uint32_t>;
				break;
			case compare_function_less_equal:
				stencil_test_[i] = compare_less_equal<uint32_t>;
				break;
			case compare_function_greater:
				stencil_test_[i] = compare_greater<uint32_t>;
				break;
			case compare_function_not_equal:
				stencil_test_[i] = compare_not_equal<uint32_t>;
				break;
			case compare_function_greater_equal:
				stencil_test_[i] = compare_greater_equal<uint32_t>;
				break;
			case compare_function_always:
				stencil_test_[i] = compare_always<uint32_t>;
				break;
			default:
				EFLIB_ASSERT(false, "");
				break;
			}
		}

		stencil_op sops[6] = 
        {
			desc.front_face.stencil_fail_op,
			desc.front_face.stencil_pass_op,
			desc.front_face.stencil_depth_fail_op,
			desc.back_face.stencil_fail_op,
			desc.back_face.stencil_pass_op,
			desc.back_face.stencil_depth_fail_op
		};

		for (int i = 0; i < 6; ++ i)
        {
			switch (sops[i])
            {
			case stencil_op_keep:
				stencil_op_[i] = sop_keep;
				break;
			case stencil_op_zero:
				stencil_op_[i] = sop_zero;
				break;
			case stencil_op_replace:
				stencil_op_[i] = sop_replace;
				break;
			case stencil_op_incr_sat:
				stencil_op_[i] = sop_incr_sat;
				break;
			case stencil_op_decr_sat:
				stencil_op_[i] = sop_decr_sat;
				break;
			case stencil_op_invert:
				stencil_op_[i] = sop_invert;
				break;
			case stencil_op_incr_wrap:
				stencil_op_[i] = sop_incr_wrap;
				break;
			case stencil_op_decr_wrap:
				stencil_op_[i] = sop_decr_wrap;
				break;
			default:
				EFLIB_ASSERT(false, "");
				break;
			}
		}
	}
	else
    {
		for (int i = 0; i < 2; ++ i)
        {
			stencil_test_[i] = compare_always<uint32_t>;
		}

		for (int i = 0; i < 6; ++ i)
        {
			stencil_op_[i] = sop_keep;
		}
	}
}

const depth_stencil_desc& depth_stencil_state::get_desc() const
{
	return desc_;
}

uint32_t depth_stencil_state::mask_stencil(uint32_t stencil, uint32_t mask) const
{
    return mask_stencil_(stencil, mask);
}

bool depth_stencil_state::depth_test(float ps_depth, float cur_depth) const
{
	return depth_test_(ps_depth, cur_depth);
}

bool depth_stencil_state::stencil_test(bool front_face, uint32_t ref, uint32_t cur_stencil) const
{
	return stencil_test_[!front_face](ref, cur_stencil);
}

uint32_t depth_stencil_state::stencil_operation(bool front_face, bool depth_pass, bool stencil_pass, uint32_t ref, uint32_t cur_stencil) const
{
	return stencil_op_[(!front_face) * 3 + (!depth_pass) + static_cast<int>(stencil_pass)](ref, cur_stencil);
}

// frame buffer

void read_depth_0_stencil_0(float& depth, uint32_t& stencil, uint32_t /*stencil_mask*/, void const* /*ds_data*/)
{
    depth = 0.0f;
    stencil = 0;
}

template <uint32_t Format>
void read_depth_0_stencil_1(float& depth, uint32_t& stencil, uint32_t stencil_mask, void const* ds_data)
{
    depth = 0.0f;
    stencil = depth_stencil_accessor<Format>::read_stencil(ds_data) & stencil_mask;
}

template <uint32_t Format>
void read_depth_1_stencil_0(float& depth, uint32_t& stencil, uint32_t /*stencil_mask*/, void const* ds_data)
{
    depth = depth_stencil_accessor<Format>::read_depth(ds_data);
    stencil = 0;
}

template <uint32_t Format>
void read_depth_1_stencil_1(float& depth, uint32_t& stencil, uint32_t stencil_mask, void const* ds_data)
{
    depth_stencil_accessor<Format>::read_depth_stencil(depth, stencil, ds_data);
    stencil &= stencil_mask;
}

void write_depth_0_stencil_0(void* /*ds_data*/, float /*depth*/, uint32_t /*stencil*/, uint32_t /*stencil_mask*/)
{
}

template <uint32_t Format>
void write_depth_0_stencil_1(void* ds_data, float /*depth*/, uint32_t stencil, uint32_t stencil_mask)
{
    depth_stencil_accessor<Format>::write_stencil(ds_data, stencil & stencil_mask);
}

template <uint32_t Format>
void write_depth_1_stencil_0(void* ds_data, float depth, uint32_t /*stencil*/, uint32_t /*stencil_mask*/)
{
    depth_stencil_accessor<Format>::write_depth(ds_data, depth);
}

template <uint32_t Format>
void write_depth_1_stencil_1(void* ds_data, float depth, uint32_t stencil, uint32_t stencil_mask)
{
    depth_stencil_accessor<Format>::write_depth_stencil(ds_data, depth, stencil & stencil_mask);
}

void framebuffer::initialize(render_stages const* /*stages*/)
{
}

void framebuffer::update(render_state* state)
{
    bool ds_state_changed = (ds_state_ == state->ds_state.get());
    bool ds_format_changed = true;
    if( ds_target_ != nullptr
        && state->depth_stencil_target.get() != nullptr
        && state->depth_stencil_target->get_pixel_format() == ds_target_->get_pixel_format()
        )
    {
        ds_format_changed = false;
    }

	ds_state_	= state->ds_state.get();
    stencil_read_mask_  = ds_state_->get_desc().stencil_read_mask;
    stencil_write_mask_ = ds_state_->get_desc().stencil_write_mask;
    stencil_ref_= ds_state_->mask_stencil(state->stencil_ref, stencil_read_mask_);
    
    assert(state->color_targets.size() <= MAX_RENDER_TARGETS);
    memset( color_targets_, 0, sizeof(color_targets_) );
    for(size_t i = 0; i < state->color_targets.size(); ++i)
    {
        color_targets_[i] = state->color_targets[i].get();
    }

    ds_target_ = state->depth_stencil_target.get();

    update_ds_rw_functions(ds_format_changed, ds_state_changed);
}

void framebuffer::update_ds_rw_functions(bool ds_format_changed, bool ds_state_changed)
{
    if(!ds_format_changed && !ds_state_changed)
    {
        return;
    }

    read_depth_stencil_  = read_depth_0_stencil_0; 
    write_depth_stencil_ = write_depth_0_stencil_0;

    if(ds_target_ == nullptr)
    {
        return;
    }

    bool read_depth = false;
    bool read_stencil = false;
    bool write_depth = false;
    bool write_stencil = false;

    switch(ds_target_->get_pixel_format())
    {
    case pixel_format_color_rg32f:
        if(ds_state_->get_desc().depth_enable)
        {
            if( ds_state_->get_desc().depth_func != compare_function_never
                && ds_state_->get_desc().depth_func != compare_function_always )
            {
                read_depth = true;
            }

            if(ds_state_->get_desc().depth_write_mask && ds_state_->get_desc().depth_func != compare_function_never)
            {
                write_depth = true;
            }
        }

        read_stencil = write_stencil = ds_state_->get_desc().stencil_enable;

        if(read_depth)
        {
            if(read_stencil)
            {
                read_depth_stencil_ = read_depth_1_stencil_1<pixel_format_color_rg32f>;
            }
            else
            {
                read_depth_stencil_ = read_depth_1_stencil_0<pixel_format_color_rg32f>;
            }
        }
        else
        {
            if(read_stencil)
            {
                read_depth_stencil_ = read_depth_0_stencil_1<pixel_format_color_rg32f>;
            }
            else
            {
                read_depth_stencil_ = read_depth_0_stencil_0;
            }
        }
        
        if(write_depth)
        {
            if(write_stencil)
            {
                write_depth_stencil_ = write_depth_1_stencil_1<pixel_format_color_rg32f>;
            }
            else
            {
                write_depth_stencil_ = write_depth_1_stencil_0<pixel_format_color_rg32f>;
            }
        }
        else
        {
            if(write_stencil)
            {
                write_depth_stencil_ = write_depth_0_stencil_1<pixel_format_color_rg32f>;
            }
            else
            {
                write_depth_stencil_ = write_depth_0_stencil_0;
            }
        }
        break;
    default:
        return;
    }
}

framebuffer::framebuffer()
{
    for(size_t i = 0; i < MAX_RENDER_TARGETS; ++i)
    {
        color_targets_[i] = nullptr;
    }

    ds_target_ = nullptr;
	ds_state_ = nullptr;
	stencil_ref_ = 0;
    stencil_read_mask_ = 0;
    stencil_write_mask_ = 0;
    
	read_depth_stencil_ = nullptr;
	write_depth_stencil_ = nullptr;
}

framebuffer::~framebuffer()
{
}

void framebuffer::render_sample(cpp_blend_shader* cpp_bs, size_t x, size_t y, size_t i_sample, const ps_output& ps, float depth)
{
	EFLIB_ASSERT(cpp_bs, "Blend shader is null or invalid.");
	if(!cpp_bs) return;

	//composing output
    pixel_accessor target_pixel(color_targets_, ds_target_);
	target_pixel.set_pos(x, y);
    
    void* ds_data = target_pixel.depth_stencil_address(i_sample);
    float       old_depth;
    uint32_t    old_stencil;
    read_depth_stencil_(old_depth, old_stencil, stencil_read_mask_, ds_data);

    bool depth_passed	= ds_state_->depth_test(depth, old_depth);
    bool stencil_passed = ds_state_->stencil_test(ps.front_face, stencil_ref_, old_stencil);

	if (depth_passed && stencil_passed)
	{
		int32_t new_stencil = ds_state_->stencil_operation(ps.front_face, depth_passed, stencil_passed, stencil_ref_, old_stencil);
		cpp_bs->execute(i_sample, target_pixel, ps);
        write_depth_stencil_(ds_data, depth, new_stencil, stencil_write_mask_);
	}
}

END_NS_SALVIAR();
