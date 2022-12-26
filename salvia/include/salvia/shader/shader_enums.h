#ifndef SALVIA_SHADER_ENUMS
#define SALVIA_SHADER_ENUMS

BEGIN_NS_SOFTART()

enum cbuffer_type{
	ct_none = 0,
	ct_buffer,
	ct_tbuffer,
	ct_resource_bind_info,
	CT_FORCE_INT32T = 0x7FFFFFFF
};

enum shader_variable_class{
	svc_none = 0,
	svc_scalar,
	svc_vector,
	svc_matrix_rows,
	svc_matrix_columns,
	svc_object,
	svc_struct,
	SVC_FORCE_INT32T = 0x7FFFFFFF
};

enum shader_variable_type{
	svt_none = 0,
	svt_void,
	svt_bool,
	svt_float,
	svt_string,
	svt_texture,
	svt_texture_1d,
	svt_texture_2d,
	svt_texture_3d,
	svt_texture_cube,
	svt_vertex_shader,
	svt_pixel_shader,
	svt_blend_shader,
	svt_uint,
	svt_uint8,
	svt_geometry_shader,
	svt_rasterizer,
	svt_depth_stencil,
	svt_blend,
	svt_buffer,
	svt_cbuffer,
	svt_tbuffer,
	svt_texture_1d_array,
	svt_texture_2d_array,
	svt_render_target_view,
	svt_depth_stencil_view,
	svt_texture_2d_ms,
	svt_texture_2d_ms_array,
	svt_texture_cube_array,
	svt_double,
	SVT_FORCE_INT32T = 0x7FFFFFFF
};

enum system_value_name{
	svn_none = 0,
	svn_undefined,
	svn_position,
	svn_clip_distance,
	svn_cull_distance,
	svn_vertex_id,
	svn_primitive_id,
	svn_instance_id,
	svn_is_front_face,
	svn_sample_index,
	svn_target,
	svn_depth,
	svn_coverage,
	SVN_FORCE_INT32_T = 0x7FFFFFFF
};

enum register_component_type{
	rct_unknown = 0,
	rct_uint32 = 1,
	rct_int32 = 2,
	rct_float32 = 3,
	RCT_FORCE_INT32_T = 0x7FFFFFFF
};

enum shader_input_type{
	sit_none = 0,
	sit_cbuffer,
	sit_tbuffer,
	sit_texture,
	sit_sampler,
	SIT_FORCE_INT32_T = 0x7FFFFFFF
};

enum shader_input_flags {
    sif_user_packed = 1 << 0,
    sif_comparison_sampler = 1 << 1,
    sif_texture_component_0 = 1 << 2,
    sif_texture_component_1 = 1 << 3,
    sif_texture_components = sif_texture_component_0 | sif_texture_component_1,
	SIF_FORCE_INT32_T = 0x7FFFFFFF
};

enum resource_return_type {
    rt_unorm = 1,
    rt_snorm = 2,
    rt_sint = 3,
    rt_uint = 4,
    rt_float = 5,
    rt_mixed = 6,
    rt_double = 7,
    rt_continued = 8,
	RT_FORCE_INT32_T = 0xFFFFFFFF
};

enum srv_dimension {
    srv_dimension_unknown = 0,
    srv_dimension_buffer = 1,
    srv_dimension_texture_1d = 2,
    srv_dimension_texture_1d_array = 3,
    srv_dimension_texture_2d = 4,
    srv_dimension_texture_2d_array = 5,
    srv_dimension_texture_2d_ms = 6,
    srv_dimension_texture_2d_ms_array = 7,
    srv_dimension_texture_3d = 8,
    srv_dimension_texture_cube = 9,
	SRV_FORCE_INT32_T = 0xFFFFFFFF
};

END_NS_SOFTART()
#endif