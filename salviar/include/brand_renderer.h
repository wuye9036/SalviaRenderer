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
EFLIB_DECLARE_CLASS_SHARED_PTR (renderer_impl2);
EFLIB_DECLARE_CLASS_SHARED_PTR (shader_object);
EFLIB_DECLARE_CLASS_SHARED_PTR (vertex_shader_unit);
EFLIB_DECLARE_CLASS_SHARED_PTR (pixel_shader_unit);
EFLIB_DECLARE_CLASS_SHARED_PTR (stream_assembler);
EFLIB_DECLARE_CLASS_SHARED_PTR (resource_manager);
EFLIB_DECLARE_STRUCT_SHARED_PTR(render_state);

class renderer_impl2
{
private:
	render_stages			stages_;

public:
	renderer_impl2			(renderer_parameters const* render_params);

	result	draw			(render_state_ptr const& state);
	result	clear_color		(render_state_ptr const& state);
	result	clear_depth		(render_state_ptr const& state);
	result	clear_stencil	(render_state_ptr const& state);
};

END_NS_SALVIAR();