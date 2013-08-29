#pragma once

#include <salviar/include/salviar_forward.h>

#include <salviar/include/enums.h>
#include <salviar/include/decl.h>
#include <salviar/include/colors.h>
#include <salviar/include/renderer_capacity.h>

#include <eflib/include/math/collision_detection.h>

#include <vector>

BEGIN_NS_SALVIAR();

struct pixel_accessor;
struct render_stages;
struct render_state;
struct renderer_parameters;

struct depth_stencil_op_desc
{
    stencil_op stencil_fail_op;
    stencil_op stencil_depth_fail_op;
    stencil_op stencil_pass_op;
    compare_function stencil_func;

	depth_stencil_op_desc()
		: stencil_fail_op(stencil_op_keep)
		, stencil_depth_fail_op(stencil_op_keep)
		, stencil_pass_op(stencil_op_keep)
		, stencil_func(compare_function_always)
    {
	}
};

struct depth_stencil_desc
{
    bool                    depth_enable;
    bool                    depth_write_mask;
    compare_function        depth_func;
    bool                    stencil_enable;
    uint8_t                 stencil_read_mask;
    uint8_t                 stencil_write_mask;
    depth_stencil_op_desc   front_face;
    depth_stencil_op_desc   back_face;

	depth_stencil_desc()
		: depth_enable(true)
        , depth_write_mask(true)
        , depth_func(compare_function_less)
        , stencil_enable(false)
        , stencil_read_mask(0xFF), stencil_write_mask(0xFF)
    {
	}
};

class depth_stencil_state
{
	depth_stencil_desc desc_;

	typedef uint32_t    (*mask_stencil_fn)   (uint32_t stencil, uint32_t mask);
	typedef bool    	(*depth_test_fn)     (float ps_depth,   float cur_depth);
	typedef bool	    (*stencil_test_fn)   (uint32_t ref,     uint32_t cur_stencil);
	typedef uint32_t    (*stencil_op_fn)     (uint32_t ref,     uint32_t cur_stencil);

    mask_stencil_fn mask_stencil_;
	depth_test_fn	depth_test_;
	stencil_test_fn	stencil_test_[2];
	stencil_op_fn	stencil_op_[6];

public:
	depth_stencil_state(const depth_stencil_desc& desc);
	const depth_stencil_desc& get_desc() const;
    
	bool        depth_test(float ps_depth, float cur_depth) const;
	bool        stencil_test(bool front_face, uint32_t ref, uint32_t cur_stencil) const;
	uint32_t    stencil_operation(bool front_face, bool depth_pass, bool stencil_pass, uint32_t ref, uint32_t cur_stencil) const;
    uint32_t    mask_stencil(uint32_t stencil, uint32_t stencil_mask) const;
};

class framebuffer
{
private:
    surface*                color_targets_[MAX_RENDER_TARGETS];
    surface*                ds_target_;
	depth_stencil_state*	ds_state_;
	uint32_t				stencil_ref_;
    uint32_t                stencil_read_mask_;
    uint32_t                stencil_write_mask_;
    
	void (*read_depth_stencil_)(float& depth, uint32_t& stencil, uint32_t stencil_mask, void const* ds_data);
	void (*write_depth_stencil_)(void* ds_data, float depth, uint32_t stencil, uint32_t stencil_mask);

    void update_ds_rw_functions(bool ds_format_changed, bool ds_state_changed);

public:
	void initialize	(render_stages const* stages);
	void update		(render_state* state);

	framebuffer();
	~framebuffer(void);

	void render_sample(cpp_blend_shader* cpp_bs, size_t x, size_t y, size_t i_sample, const ps_output& ps, float depth);
    // bool check_z(size_t x, size_t y, float depth);
};

END_NS_SALVIAR();