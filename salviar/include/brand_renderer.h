#pragma once

#include <salviar/include/salviar_forward.h>

#include <salviar/include/renderer.h>
#include <salviar/include/render_state.h>

#include <eflib/include/utility/shared_declaration.h>

BEGIN_NS_SALVIAR();

struct vs_input_op;
struct vs_output_op;

EFLIB_DECLARE_CLASS_SHARED_PTR(host);
EFLIB_DECLARE_CLASS_SHARED_PTR(renderer_impl2);
EFLIB_DECLARE_CLASS_SHARED_PTR(shader_object);
EFLIB_DECLARE_CLASS_SHARED_PTR(vertex_shader_unit);
EFLIB_DECLARE_CLASS_SHARED_PTR(pixel_shader_unit);
EFLIB_DECLARE_CLASS_SHARED_PTR(stream_assembler);
EFLIB_DECLARE_CLASS_SHARED_PTR(resource_manager);
EFLIB_DECLARE_STRUCT_SHARED_PTR(render_state);

class renderer_impl2
{
	render_state_ptr		state_;
	
	stream_assembler_ptr	assembler_;
	clipper_ptr				clipper_;
	vertex_cache_ptr		vertex_cache_;
	rasterizer_ptr			rast_;
	host_ptr				host_;
	framebuffer_ptr			frame_buffer_;

public:
	renderer_impl2();

	result	draw(render_state_ptr const& state);

	result	clear_color(size_t target_index, color_rgba32f const& c);
	result	clear_depth(float d);
	result	clear_stencil(uint32_t s);
};

END_NS_SALVIAR();