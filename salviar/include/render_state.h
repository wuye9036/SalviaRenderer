#pragma once

#include <salviar/include/salviar_forward.h>

#include <salviar/include/enums.h>
#include <salviar/include/format.h>
#include <salviar/include/viewport.h>
#include <salviar/include/stream_state.h>

#include <eflib/include/utility/shared_declaration.h>
#include <eflib/include/math/vector.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

BEGIN_NS_SALVIAR();

EFLIB_DECLARE_CLASS_SHARED_PTR(buffer);
EFLIB_DECLARE_CLASS_SHARED_PTR(surface);
EFLIB_DECLARE_CLASS_SHARED_PTR(input_layout);
EFLIB_DECLARE_CLASS_SHARED_PTR(counter);
EFLIB_DECLARE_CLASS_SHARED_PTR(depth_stencil_state);
EFLIB_DECLARE_CLASS_SHARED_PTR(raster_state);
EFLIB_DECLARE_CLASS_SHARED_PTR(shader_object);
EFLIB_DECLARE_CLASS_SHARED_PTR(cpp_blend_shader);
EFLIB_DECLARE_CLASS_SHARED_PTR(cpp_pixel_shader);
EFLIB_DECLARE_CLASS_SHARED_PTR(cpp_vertex_shader);
EFLIB_DECLARE_CLASS_SHARED_PTR(vertex_shader_unit);
EFLIB_DECLARE_CLASS_SHARED_PTR(pixel_shader_unit);
EFLIB_DECLARE_CLASS_SHARED_PTR(shader_cbuffer_impl);
EFLIB_DECLARE_STRUCT_SHARED_PTR(stream_state);

struct vs_input_op;
struct vs_output_op;

struct render_state
{
	buffer_ptr					index_buffer;
	format						index_format;
	primitive_topology			prim_topo;
	int32_t						base_vertex;
	uint32_t					start_index;
	uint32_t					prim_count;
	
	stream_state_ptr			str_state;
	input_layout_ptr			layout;

	viewport					vp;
	raster_state_ptr			ras_state;

	int32_t						stencil_ref;
	depth_stencil_state_ptr		ds_state;

	cpp_vertex_shader_ptr		cpp_vs;
	cpp_pixel_shader_ptr		cpp_ps;
	cpp_blend_shader_ptr		cpp_bs;

	shader_object_ptr			vx_shader;
	shader_object_ptr			px_shader;

	shader_cbuffer_impl_ptr		vx_cbuffer;
	shader_cbuffer_impl_ptr		px_cbuffer;

	vs_input_op*				vsi_ops;
	vs_output_op*				vso_ops;

	vertex_shader_unit_ptr		vs_proto;
	pixel_shader_unit_ptr		ps_proto;

	std::vector<surface_ptr>	color_targets;
	surface_ptr					depth_stencil_target;

	counter_ptr					counters[counter_ids::counter_count];

	surface_ptr					clear_target;
	float						clear_z;
	uint32_t					clear_stencil;
	eflib::vec4					clear_color;
};

END_NS_SALVIAR();
