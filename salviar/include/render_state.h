#pragma once

#include <salviar/include/salviar_forward.h>

#include <salviar/include/format.h>
#include <eflib/include/utility/shared_declaration.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

BEGIN_NS_SALVIAR();

EFLIB_DECLARE_CLASS_SHARED_PTR(buffer);
EFLIB_DECLARE_CLASS_SHARED_PTR(input_layout);
EFLIB_DECLARE_CLASS_SHARED_PTR(depth_stencil_state);
EFLIB_DECLARE_CLASS_SHARED_PTR(raster_state);
EFLIB_DECLARE_CLASS_SHARED_PTR(stream_state);
EFLIB_DECLARE_CLASS_SHARED_PTR(shader_object);
EFLIB_DECLARE_CLASS_SHARED_PTR(blend_shader);
EFLIB_DECLARE_CLASS_SHARED_PTR(pixel_shader);
EFLIB_DECLARE_CLASS_SHARED_PTR(vertex_shader);

struct vs_input_op;
struct vs_output_op;

struct render_state
{
	buffer_ptr				index_buffer;
	format					index_format;
	primitive_topology		prim_topo;
	size_t					start_index;
	size_t					draw_prim_count;
	
	stream_state_ptr		stream_states;
	input_layout_ptr		layout;

	viewport				vp;
	raster_state_ptr		ras_state;

	int32_t					stencil_ref;
	depth_stencil_state_ptr	ds_state;

	// Shader as states
	vertex_shader_ptr		cpp_vs;
	pixel_shader_ptr		cpp_ps;
	blend_shader_ptr		cpp_bs;

	shader_object_ptr		vx_shader;
	shader_object_ptr		px_shader;

	vs_input_op*			vsi_ops;
	vs_output_op*			vso_ops;

	vertex_shader_unit_ptr	vs_proto_;
	pixel_shader_unit_ptr	ps_proto_;
};

END_NS_SALVIAR();
