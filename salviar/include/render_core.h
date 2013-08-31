#pragma once

#include <salviar/include/salviar_forward.h>

#include <salviar/include/render_state.h>
#include <salviar/include/render_stages.h>

#include <eflib/include/utility/shared_declaration.h>

BEGIN_NS_SALVIAR();

struct vs_input_op;
struct vs_output_op;
struct renderer_parameters;

EFLIB_DECLARE_CLASS_SHARED_PTR (host);
EFLIB_DECLARE_CLASS_SHARED_PTR (render_core);
EFLIB_DECLARE_CLASS_SHARED_PTR (shader_object);
EFLIB_DECLARE_CLASS_SHARED_PTR (vertex_shader_unit);
EFLIB_DECLARE_CLASS_SHARED_PTR (pixel_shader_unit);
EFLIB_DECLARE_CLASS_SHARED_PTR (stream_assembler);
EFLIB_DECLARE_CLASS_SHARED_PTR (resource_manager);
EFLIB_DECLARE_STRUCT_SHARED_PTR(render_state);

class render_core
{
private:

public:
	render_core		();
    void    update  (render_state_ptr const& state);
    result  execute ();

private:
    render_stages		stages_;
    render_state_ptr	state_;

	result	draw();
	result	clear_color();
	result	clear_depth_stencil();
    void    apply_shader_cbuffer();
    result  async_start();
    result  async_stop();
};

END_NS_SALVIAR();