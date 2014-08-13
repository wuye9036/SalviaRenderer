#pragma once

#include <salviar/include/renderer.h>

class umd_resource;

class umd_device
{
	friend umd_resource;

public:
	umd_device(umd_adapter* adapter, const D3D10DDIARG_CREATEDEVICE* args);
	~umd_device();

	void destroy();

	void set_d3d_error(HRESULT hr);

	D3DKMT_CREATEDCFROMMEMORY& dc_from_mem()
	{
		return dc_from_mem_;
	}
	const salviar::surface_ptr& resolved_surf() const
	{
		return resolved_surf_;
	}

private:
	static HRESULT APIENTRY retrieve_sub_object(D3D10DDI_HDEVICE device, UINT32 sub_device_id,
		SIZE_T param_size, void* params, SIZE_T output_param_size, void* output_params_buffer);

	static void APIENTRY default_constant_buffer_update_subresource_up(D3D10DDI_HDEVICE device,
		D3D10DDI_HRESOURCE dst_resource, UINT dst_subresource, const D3D10_DDI_BOX* dst_box,
		const void* sys_mem_up, UINT row_pitch, UINT depth_pitch);
	static void APIENTRY vs_set_constant_buffers(D3D10DDI_HDEVICE device, UINT start_buffer,
		UINT num_buffers, const D3D10DDI_HRESOURCE* buffers);
	static void APIENTRY ps_set_shader_resources(D3D10DDI_HDEVICE device, UINT offset,
		UINT num_views, const D3D10DDI_HSHADERRESOURCEVIEW* srvs);
	static void APIENTRY ps_set_shader(D3D10DDI_HDEVICE device, D3D10DDI_HSHADER shader);
	static void APIENTRY ps_set_samplers(D3D10DDI_HDEVICE device, UINT offset,
		UINT num_samplers, const D3D10DDI_HSAMPLER* samplers);
	static void APIENTRY vs_set_shader(D3D10DDI_HDEVICE device, D3D10DDI_HSHADER shader);
	static void APIENTRY draw_indexed(D3D10DDI_HDEVICE device, UINT index_count,
		UINT start_index_location, INT base_vertex_location);
	static void APIENTRY draw(D3D10DDI_HDEVICE device, UINT vertex_count,
		UINT start_vertex_location);
	static void APIENTRY dynamic_ia_buffer_map_no_overwrite(D3D10DDI_HDEVICE device, D3D10DDI_HRESOURCE resource,
		UINT subresource, D3D10_DDI_MAP ddi_map, UINT flags,
		D3D10DDI_MAPPED_SUBRESOURCE* mapped_subresource);
	static void APIENTRY dynamic_ia_buffer_unmap(D3D10DDI_HDEVICE device, D3D10DDI_HRESOURCE resource,
		UINT subresource);
	static void APIENTRY dynamic_constant_buffer_map_discard(D3D10DDI_HDEVICE device, D3D10DDI_HRESOURCE resource,
		UINT subresource, D3D10_DDI_MAP ddi_map, UINT flags,
		D3D10DDI_MAPPED_SUBRESOURCE* mapped_subresource);
	static void APIENTRY dynamic_ia_buffer_map_discard(D3D10DDI_HDEVICE device, D3D10DDI_HRESOURCE resource,
		UINT subresource, D3D10_DDI_MAP ddi_map, UINT flags,
		D3D10DDI_MAPPED_SUBRESOURCE* mapped_subresource);
	static void APIENTRY dynamic_constant_buffer_unmap(D3D10DDI_HDEVICE device, D3D10DDI_HRESOURCE resource,
		UINT subresource);
	static void APIENTRY ps_set_constant_buffers(D3D10DDI_HDEVICE device, UINT start_buffer,
		UINT num_buffers, const D3D10DDI_HRESOURCE* buffers);
	static void APIENTRY ia_set_input_layout(D3D10DDI_HDEVICE device,
		D3D10DDI_HELEMENTLAYOUT input_layout);
	static void APIENTRY ia_set_vertex_buffers(D3D10DDI_HDEVICE device, UINT start_buffer,
		UINT num_buffers, const D3D10DDI_HRESOURCE* buffers, const UINT* strides, const UINT* offsets);
	static void APIENTRY ia_set_index_buffer(D3D10DDI_HDEVICE device, D3D10DDI_HRESOURCE buffer,
		DXGI_FORMAT format, UINT offset);

	static void APIENTRY draw_indexed_instanced(D3D10DDI_HDEVICE device, UINT index_count_per_instance,
		UINT instance_count, UINT start_index_location, INT base_vertex_location, UINT start_instance_location);
	static void APIENTRY draw_instanced(D3D10DDI_HDEVICE device, UINT vertex_count_per_instance,
		UINT instance_count, UINT start_vertex_location, UINT start_instance_location);
	static void APIENTRY dynamic_resource_map_discard(D3D10DDI_HDEVICE device, D3D10DDI_HRESOURCE resource,
		UINT subresource, D3D10_DDI_MAP ddi_map, UINT flags,
		D3D10DDI_MAPPED_SUBRESOURCE* mapped_subresource);
	static void APIENTRY dynamic_resource_unmap(D3D10DDI_HDEVICE device, D3D10DDI_HRESOURCE resource,
		UINT subresource);
	static void APIENTRY gs_set_constant_buffers(D3D10DDI_HDEVICE device, UINT start_buffer,
		UINT num_buffers, const D3D10DDI_HRESOURCE* buffers);
	static void APIENTRY gs_set_shader(D3D10DDI_HDEVICE device, D3D10DDI_HSHADER shader);
	static void APIENTRY ia_set_topology(D3D10DDI_HDEVICE device, D3D10_DDI_PRIMITIVE_TOPOLOGY primitive_topology);
	static void APIENTRY staging_resource_map(D3D10DDI_HDEVICE device, D3D10DDI_HRESOURCE resource,
		UINT subresource, D3D10_DDI_MAP ddi_map, UINT flags,
		D3D10DDI_MAPPED_SUBRESOURCE* mapped_subresource);
	static void APIENTRY staging_resource_unmap(D3D10DDI_HDEVICE device, D3D10DDI_HRESOURCE resource,
		UINT subresource);
	static void APIENTRY vs_set_shader_resources(D3D10DDI_HDEVICE device, UINT offset, UINT num_views,
		const D3D10DDI_HSHADERRESOURCEVIEW* shader_resource_views);
	static void APIENTRY vs_set_samplers(D3D10DDI_HDEVICE device, UINT offset, UINT num_samplers,
		const D3D10DDI_HSAMPLER* samplers);
	static void APIENTRY gs_set_shader_resources(D3D10DDI_HDEVICE device, UINT offset, UINT num_views,
		const D3D10DDI_HSHADERRESOURCEVIEW* shader_resource_views);
	static void APIENTRY gs_set_samplers(D3D10DDI_HDEVICE device, UINT offset, UINT num_samplers,
		const D3D10DDI_HSAMPLER* samplers);
	static void APIENTRY set_render_targets(D3D10DDI_HDEVICE device,
		const D3D10DDI_HRENDERTARGETVIEW* render_target_view, UINT num_rtvs, UINT rtv_number_to_unbind,
		D3D10DDI_HDEPTHSTENCILVIEW depth_stencil_view, const D3D11DDI_HUNORDEREDACCESSVIEW* unordered_access_view,
		const UINT* uav_initial_counts, UINT uav_index, UINT num_uavs, UINT uav_first_to_set, UINT uav_number_updated);
	static void APIENTRY shader_resource_view_read_after_write_hazard(D3D10DDI_HDEVICE device,
		D3D10DDI_HSHADERRESOURCEVIEW shader_resource_view, D3D10DDI_HRESOURCE resource);
	static void APIENTRY resource_read_after_write_hazard(D3D10DDI_HDEVICE device, D3D10DDI_HRESOURCE resource);
	static void APIENTRY set_blend_state(D3D10DDI_HDEVICE device, D3D10DDI_HBLENDSTATE state,
		const FLOAT blend_factor[4], UINT sample_mask);
	static void APIENTRY set_depth_stencil_state(D3D10DDI_HDEVICE device, D3D10DDI_HDEPTHSTENCILSTATE state,
		UINT stencil_ref);
	static void APIENTRY set_rasterizer_state(D3D10DDI_HDEVICE device, D3D10DDI_HRASTERIZERSTATE state);
	static void APIENTRY query_end(D3D10DDI_HDEVICE device, D3D10DDI_HQUERY query);
	static void APIENTRY query_begin(D3D10DDI_HDEVICE device, D3D10DDI_HQUERY query);
	static void APIENTRY resource_copy_region(D3D10DDI_HDEVICE device, D3D10DDI_HRESOURCE dst_resource, UINT dst_subresource,
		UINT dst_x, UINT dst_y, UINT dst_z, D3D10DDI_HRESOURCE src_resource, UINT src_subresource, const D3D10_DDI_BOX* src_box);
	static void APIENTRY resource_update_subresource_up(D3D10DDI_HDEVICE device, D3D10DDI_HRESOURCE dst_resource,
		UINT dst_subresource, const D3D10_DDI_BOX* dst_box, const VOID* sys_mem_up, UINT row_pitch, UINT depth_pitch);
	static void APIENTRY so_set_targets(D3D10DDI_HDEVICE device, UINT so_targets, UINT clear_targets,
		const D3D10DDI_HRESOURCE* resource, const UINT* offsets);
	static void APIENTRY draw_auto(D3D10DDI_HDEVICE device);
	static void APIENTRY set_viewports(D3D10DDI_HDEVICE device, UINT num_viewports, UINT clear_viewports,
		const D3D10_DDI_VIEWPORT* viewports);
	static void APIENTRY set_scissor_rects(D3D10DDI_HDEVICE device, UINT num_scissor_rects, UINT clear_scissor_rects,
		const D3D10_DDI_RECT* rects);
	static void APIENTRY clear_render_target_view(D3D10DDI_HDEVICE device, D3D10DDI_HRENDERTARGETVIEW render_target_view,
		FLOAT color_rgba[4]);
	static void APIENTRY clear_depth_stencil_view(D3D10DDI_HDEVICE device, D3D10DDI_HDEPTHSTENCILVIEW depth_stencil_view,
		UINT flags, FLOAT depth, UINT8 stencil);
	static void APIENTRY set_predication(D3D10DDI_HDEVICE device, D3D10DDI_HQUERY query, BOOL predicate_value);
	static void APIENTRY query_get_data(D3D10DDI_HDEVICE device, D3D10DDI_HQUERY query, VOID* data, UINT data_size, UINT flags);
	static void APIENTRY flush(D3D10DDI_HDEVICE device);
	static void APIENTRY gen_mips(D3D10DDI_HDEVICE device, D3D10DDI_HSHADERRESOURCEVIEW shader_resource_view);
	static void APIENTRY resource_copy(D3D10DDI_HDEVICE device, D3D10DDI_HRESOURCE dst_resource, D3D10DDI_HRESOURCE src_resource);
	static void APIENTRY resource_resolve_subresource(D3D10DDI_HDEVICE device, D3D10DDI_HRESOURCE dst_resource,
		UINT dst_subresource, D3D10DDI_HRESOURCE src_resource, UINT src_subresource, DXGI_FORMAT resolve_format);

	static void APIENTRY resource_map(D3D10DDI_HDEVICE device, D3D10DDI_HRESOURCE resource,
		UINT subresource, D3D10_DDI_MAP ddi_map, UINT flags,
		D3D10DDI_MAPPED_SUBRESOURCE* mapped_subresource);
	static void APIENTRY resource_unmap(D3D10DDI_HDEVICE device, D3D10DDI_HRESOURCE resource,
		UINT subresource);
	static BOOL APIENTRY resource_is_staging_busy(D3D10DDI_HDEVICE device, D3D10DDI_HRESOURCE resource);
	static void APIENTRY relocate_device_funcs(D3D10DDI_HDEVICE device, D3D11DDI_DEVICEFUNCS* device_funcs);
	static SIZE_T APIENTRY calc_private_resource_size(D3D10DDI_HDEVICE device, const D3D11DDIARG_CREATERESOURCE* create_resource);
	static SIZE_T APIENTRY calc_private_opened_resource_size(D3D10DDI_HDEVICE device, const D3D10DDIARG_OPENRESOURCE* open_resource);
	static void APIENTRY create_resource(D3D10DDI_HDEVICE device, const D3D11DDIARG_CREATERESOURCE* create_resource,
		D3D10DDI_HRESOURCE resource, D3D10DDI_HRTRESOURCE rt_resource);
	static void APIENTRY open_resource(D3D10DDI_HDEVICE device, const D3D10DDIARG_OPENRESOURCE* open_resource,
		D3D10DDI_HRESOURCE resource, D3D10DDI_HRTRESOURCE rt_resource);
	static void APIENTRY destroy_resource(D3D10DDI_HDEVICE device, D3D10DDI_HRESOURCE resource);
	static SIZE_T APIENTRY calc_private_shader_resource_view_size(D3D10DDI_HDEVICE device,
		const D3D11DDIARG_CREATESHADERRESOURCEVIEW* create_shader_resource_view);
	static void APIENTRY create_shader_resource_view(D3D10DDI_HDEVICE device,
		const D3D11DDIARG_CREATESHADERRESOURCEVIEW* create_shader_resource_view,
		D3D10DDI_HSHADERRESOURCEVIEW shader_resource_view, D3D10DDI_HRTSHADERRESOURCEVIEW rt_shader_resource_view);
	static void APIENTRY destroy_shader_resource_view(D3D10DDI_HDEVICE device,
		D3D10DDI_HSHADERRESOURCEVIEW shader_resource_view);
	static SIZE_T APIENTRY calc_private_render_target_view_size(D3D10DDI_HDEVICE device,
		const D3D10DDIARG_CREATERENDERTARGETVIEW* create_render_target_view);
	static void APIENTRY create_render_target_view(D3D10DDI_HDEVICE device,
		const D3D10DDIARG_CREATERENDERTARGETVIEW* create_render_target_view, D3D10DDI_HRENDERTARGETVIEW render_target_view,
		D3D10DDI_HRTRENDERTARGETVIEW rt_render_target_view);
	static void APIENTRY destroy_render_target_view(D3D10DDI_HDEVICE device, D3D10DDI_HRENDERTARGETVIEW render_target_view);
	static SIZE_T APIENTRY calc_private_depth_stencil_view_size(D3D10DDI_HDEVICE device,
		const D3D11DDIARG_CREATEDEPTHSTENCILVIEW* create_depth_stencil_view);
	static void APIENTRY create_depth_stencil_view(D3D10DDI_HDEVICE device,
		const D3D11DDIARG_CREATEDEPTHSTENCILVIEW* create_depth_stencil_view, D3D10DDI_HDEPTHSTENCILVIEW depth_stencil_view,
		D3D10DDI_HRTDEPTHSTENCILVIEW rt_depth_stencil_view);
	static void APIENTRY destroy_depth_stencil_view(D3D10DDI_HDEVICE device, D3D10DDI_HDEPTHSTENCILVIEW depth_stencil_view);
	static SIZE_T APIENTRY calc_private_element_layout_size(D3D10DDI_HDEVICE device,
		const D3D10DDIARG_CREATEELEMENTLAYOUT* create_element_layout);
	static void APIENTRY create_element_layout(D3D10DDI_HDEVICE device,
		const D3D10DDIARG_CREATEELEMENTLAYOUT* create_element_layout, D3D10DDI_HELEMENTLAYOUT element_layout,
		D3D10DDI_HRTELEMENTLAYOUT rt_element_layout);
	static void APIENTRY destroy_element_layout(D3D10DDI_HDEVICE device, D3D10DDI_HELEMENTLAYOUT element_layout);
	static SIZE_T APIENTRY calc_private_blend_state_size(D3D10DDI_HDEVICE device,
		const D3D10_1_DDI_BLEND_DESC* blend_desc);
	static void APIENTRY create_blend_state(D3D10DDI_HDEVICE device,
		const D3D10_1_DDI_BLEND_DESC* blend_desc, D3D10DDI_HBLENDSTATE blend_state,
		D3D10DDI_HRTBLENDSTATE rt_blend_state);
	static void APIENTRY destroy_blend_state(D3D10DDI_HDEVICE device, D3D10DDI_HBLENDSTATE blend_state);
	static SIZE_T APIENTRY calc_private_depth_stencil_state_size(D3D10DDI_HDEVICE device,
		const D3D10_DDI_DEPTH_STENCIL_DESC* depth_stencil_desc);
	static void APIENTRY create_depth_stencil_state(D3D10DDI_HDEVICE device,
		const D3D10_DDI_DEPTH_STENCIL_DESC* depth_stencil_desc, D3D10DDI_HDEPTHSTENCILSTATE depth_stencil_state,
		D3D10DDI_HRTDEPTHSTENCILSTATE rt_depth_stencil_state);
	static void APIENTRY destroy_depth_stencil_state(D3D10DDI_HDEVICE device, D3D10DDI_HDEPTHSTENCILSTATE depth_stencil_state);
	static SIZE_T APIENTRY calc_private_rasterizer_stateSize(D3D10DDI_HDEVICE device,
		const D3D10_DDI_RASTERIZER_DESC* rasterizer_desc);
	static void APIENTRY create_rasterizer_state(D3D10DDI_HDEVICE device,
		const D3D10_DDI_RASTERIZER_DESC* rasterizer_desc, D3D10DDI_HRASTERIZERSTATE rasterizer_state,
		D3D10DDI_HRTRASTERIZERSTATE rt_rasterizer_state);
	static void APIENTRY destroy_rasterizer_state(D3D10DDI_HDEVICE device, D3D10DDI_HRASTERIZERSTATE rasterizer_state);
	static SIZE_T APIENTRY calc_private_shader_size(D3D10DDI_HDEVICE device, const UINT* code,
		const D3D10DDIARG_STAGE_IO_SIGNATURES* signatures);
	static void APIENTRY create_vertex_shader(D3D10DDI_HDEVICE device, const UINT* code,
		D3D10DDI_HSHADER shader, D3D10DDI_HRTSHADER rt_shader, const D3D10DDIARG_STAGE_IO_SIGNATURES* signatures);
	static void APIENTRY create_geometry_shader(D3D10DDI_HDEVICE device, const UINT* code,
		D3D10DDI_HSHADER shader, D3D10DDI_HRTSHADER rt_shader, const D3D10DDIARG_STAGE_IO_SIGNATURES* signatures);
	static void APIENTRY create_pixel_shader(D3D10DDI_HDEVICE device, const UINT* code,
		D3D10DDI_HSHADER shader, D3D10DDI_HRTSHADER rt_shader, const D3D10DDIARG_STAGE_IO_SIGNATURES* signatures);
	static SIZE_T APIENTRY calc_private_geometry_shader_with_stream_output(D3D10DDI_HDEVICE device,
		const D3D11DDIARG_CREATEGEOMETRYSHADERWITHSTREAMOUTPUT* create_geometry_shader_with_stream_output,
		const D3D10DDIARG_STAGE_IO_SIGNATURES* signatures);
	static void APIENTRY create_geometry_shader_with_stream_output(D3D10DDI_HDEVICE device,
		const D3D11DDIARG_CREATEGEOMETRYSHADERWITHSTREAMOUTPUT* create_geometry_shader_with_stream_output,
		D3D10DDI_HSHADER shader, D3D10DDI_HRTSHADER rt_shader, const D3D10DDIARG_STAGE_IO_SIGNATURES* signatures);
	static void APIENTRY destroy_shader(D3D10DDI_HDEVICE device, D3D10DDI_HSHADER shader);
	static SIZE_T APIENTRY calc_private_sampler_size(D3D10DDI_HDEVICE device,
		const D3D10_DDI_SAMPLER_DESC* sampler_desc);
	static void APIENTRY create_sampler(D3D10DDI_HDEVICE device,
		const D3D10_DDI_SAMPLER_DESC* sampler_desc, D3D10DDI_HSAMPLER sampler,
		D3D10DDI_HRTSAMPLER rt_sampler);
	static void APIENTRY destroy_sampler(D3D10DDI_HDEVICE device, D3D10DDI_HSAMPLER sampler);
	static SIZE_T APIENTRY calc_private_query_size(D3D10DDI_HDEVICE device,
		const D3D10DDIARG_CREATEQUERY* create_query);
	static void APIENTRY create_query(D3D10DDI_HDEVICE device,
		const D3D10DDIARG_CREATEQUERY *create_query, D3D10DDI_HQUERY query,
		D3D10DDI_HRTQUERY rt_query);
	static void APIENTRY destroy_query(D3D10DDI_HDEVICE device, D3D10DDI_HQUERY query);
	static void APIENTRY check_format_support(D3D10DDI_HDEVICE device,
		DXGI_FORMAT format, UINT* format_caps);
	static void APIENTRY check_multisample_quality_levels(D3D10DDI_HDEVICE device,
		DXGI_FORMAT format, UINT sample_count, UINT* num_quality_levels);
	static void APIENTRY check_counter_info(D3D10DDI_HDEVICE device,
		D3D10DDI_COUNTER_INFO* counter_info);
	static void APIENTRY check_counter(D3D10DDI_HDEVICE device,
		D3D10DDI_QUERY query,  D3D10DDI_COUNTER_TYPE* counter_type,
		UINT* active_counters, LPSTR name, UINT* name_length, LPSTR units, UINT* units_length,
		LPSTR description, UINT* description_length);
	static void APIENTRY destroy_device(D3D10DDI_HDEVICE device);
	static void APIENTRY set_text_filter_size(D3D10DDI_HDEVICE device,
		UINT width, UINT height);

	static void APIENTRY resource_convert(D3D10DDI_HDEVICE device, D3D10DDI_HRESOURCE dst_resource,
		D3D10DDI_HRESOURCE src_resource);
	static void APIENTRY resource_convert_region(D3D10DDI_HDEVICE device, D3D10DDI_HRESOURCE dst_resource,
		UINT dst_subresource, UINT dst_x, UINT dst_y, UINT dst_z,
		D3D10DDI_HRESOURCE src_resource, UINT src_subresource, const D3D10_DDI_BOX* src_box);

	static void APIENTRY draw_indexed_instanced_indirect(D3D10DDI_HDEVICE device,
		D3D10DDI_HRESOURCE buffer_for_args, UINT aligned_byte_offset_for_args);
	static void APIENTRY draw_instanced_indirect(D3D10DDI_HDEVICE device,
		D3D10DDI_HRESOURCE buffer_for_args, UINT aligned_byte_offset_for_args);
	static void APIENTRY command_list_execute(D3D10DDI_HDEVICE device,
		D3D11DDI_HCOMMANDLIST command_list);
	static void APIENTRY hs_set_shader_resources(D3D10DDI_HDEVICE device, UINT offset, UINT num_views,
		const D3D10DDI_HSHADERRESOURCEVIEW* shader_resource_views);
	static void APIENTRY hs_set_shader(D3D10DDI_HDEVICE device, D3D10DDI_HSHADER shader);
	static void APIENTRY hs_set_samplers(D3D10DDI_HDEVICE device, UINT offset, UINT num_samplers,
		const D3D10DDI_HSAMPLER* samplers);
	static void APIENTRY hs_set_constant_buffers(D3D10DDI_HDEVICE device, UINT start_buffer, UINT num_buffers,
		const D3D10DDI_HRESOURCE* buffers);
	static void APIENTRY ds_set_shader_resources(D3D10DDI_HDEVICE device, UINT offset, UINT num_views,
		const D3D10DDI_HSHADERRESOURCEVIEW *shader_resource_views);
	static void APIENTRY ds_set_shader(D3D10DDI_HDEVICE device, D3D10DDI_HSHADER shader);
	static void APIENTRY ds_set_samplers(D3D10DDI_HDEVICE device, UINT offset, UINT num_samplers,
		const D3D10DDI_HSAMPLER* samplers);
	static void APIENTRY ds_set_constant_buffers(D3D10DDI_HDEVICE device, UINT start_buffer, UINT num_buffers,
		const D3D10DDI_HRESOURCE* buffers);
	static void APIENTRY create_hull_shader(D3D10DDI_HDEVICE device, const UINT* code,
		D3D10DDI_HSHADER shader, D3D10DDI_HRTSHADER rt_shader, const D3D11DDIARG_TESSELLATION_IO_SIGNATURES* signatures);
	static void APIENTRY create_domain_shader(D3D10DDI_HDEVICE device, const UINT* code,
		D3D10DDI_HSHADER shader, D3D10DDI_HRTSHADER rt_shader, const D3D11DDIARG_TESSELLATION_IO_SIGNATURES* signatures);
	static void APIENTRY check_deferred_context_handle_sizes(D3D10DDI_HDEVICE device, UINT* h_sizes,
		D3D11DDI_HANDLESIZE* handle_size);
	static SIZE_T APIENTRY calc_deferred_context_handle_size(D3D10DDI_HDEVICE device, D3D11DDI_HANDLETYPE handle_type,
		void* ic_object);
	static SIZE_T APIENTRY calc_private_deferred_context_size(D3D10DDI_HDEVICE device,
		const D3D11DDIARG_CALCPRIVATEDEFERREDCONTEXTSIZE* calc_private_deferred_context_size);
	static void APIENTRY create_deferred_context(D3D10DDI_HDEVICE device,
		const D3D11DDIARG_CREATEDEFERREDCONTEXT* create_deferred_context);
	static void APIENTRY abandon_command_list(D3D10DDI_HDEVICE device);
	static SIZE_T APIENTRY calc_private_command_list_size(D3D10DDI_HDEVICE device,
		const D3D11DDIARG_CREATECOMMANDLIST* create_command_list);
	static void APIENTRY create_command_list(D3D10DDI_HDEVICE device,
		const D3D11DDIARG_CREATECOMMANDLIST* create_command_list, D3D11DDI_HCOMMANDLIST command_list,
		D3D11DDI_HRTCOMMANDLIST rt_command_list);
	static void APIENTRY destroy_command_list(D3D10DDI_HDEVICE device, D3D11DDI_HCOMMANDLIST command_list);
	static SIZE_T APIENTRY calc_private_tessellation_shader_size(D3D10DDI_HDEVICE device, const UINT* code,
		const D3D11DDIARG_TESSELLATION_IO_SIGNATURES* signatures);
	static void APIENTRY ps_set_shader_with_ifaces(D3D10DDI_HDEVICE device, D3D10DDI_HSHADER shader,
		UINT num_class_instances, const UINT* ifaces, const D3D11DDIARG_POINTERDATA* pointer_data);
	static void APIENTRY vs_set_shader_with_ifaces(D3D10DDI_HDEVICE device, D3D10DDI_HSHADER shader,
		UINT num_class_instances, const UINT* ifaces, const D3D11DDIARG_POINTERDATA* pointer_data);
	static void APIENTRY gs_set_shader_with_ifaces(D3D10DDI_HDEVICE device, D3D10DDI_HSHADER shader,
		UINT num_class_instances, const UINT* ifaces, const D3D11DDIARG_POINTERDATA* pointer_data);
	static void APIENTRY hs_set_shader_with_ifaces(D3D10DDI_HDEVICE device, D3D10DDI_HSHADER shader,
		UINT num_class_instances, const UINT* ifaces, const D3D11DDIARG_POINTERDATA* pointer_data);
	static void APIENTRY ds_set_shader_with_ifaces(D3D10DDI_HDEVICE device, D3D10DDI_HSHADER shader,
		UINT num_class_instances, const UINT* ifaces, const D3D11DDIARG_POINTERDATA* pointer_data);
	static void APIENTRY cs_set_shader_with_ifaces(D3D10DDI_HDEVICE device, D3D10DDI_HSHADER shader,
		UINT num_class_instances, const UINT* ifaces, const D3D11DDIARG_POINTERDATA* pointer_data);
	static void APIENTRY create_compute_shader(D3D10DDI_HDEVICE device, const UINT* code,
		D3D10DDI_HSHADER shader, D3D10DDI_HRTSHADER rt_shader);
	static void APIENTRY cs_set_shader(D3D10DDI_HDEVICE device, D3D10DDI_HSHADER shader);
	static void APIENTRY cs_set_shader_resources(D3D10DDI_HDEVICE device, UINT offset, UINT num_views,
		const D3D10DDI_HSHADERRESOURCEVIEW* shader_resource_views);
	static void APIENTRY cs_set_samplers(D3D10DDI_HDEVICE device, UINT offset, UINT num_samplers,
		const D3D10DDI_HSAMPLER* samplers);
	static void APIENTRY cs_set_constant_buffers(D3D10DDI_HDEVICE device, UINT start_buffer, UINT num_buffers,
		const D3D10DDI_HRESOURCE* buffers);
	static SIZE_T APIENTRY calc_private_unordered_access_view_size(D3D10DDI_HDEVICE device,
		const D3D11DDIARG_CREATEUNORDEREDACCESSVIEW* create_unordered_access_view);
	static void APIENTRY create_unordered_access_view(D3D10DDI_HDEVICE device,
		const D3D11DDIARG_CREATEUNORDEREDACCESSVIEW* create_unordered_access_view,
		D3D11DDI_HUNORDEREDACCESSVIEW unordered_access_view, D3D11DDI_HRTUNORDEREDACCESSVIEW rt_unordered_access_view);
	static void APIENTRY destroy_unordered_access_view(D3D10DDI_HDEVICE device,
		D3D11DDI_HUNORDEREDACCESSVIEW unordered_access_view);
	static void APIENTRY clear_unordered_access_view_uint(D3D10DDI_HDEVICE device,
		D3D11DDI_HUNORDEREDACCESSVIEW unordered_access_view, const UINT uints[4]);
	static void APIENTRY clear_unordered_access_view_float(D3D10DDI_HDEVICE device,
		D3D11DDI_HUNORDEREDACCESSVIEW unordered_access_view, const FLOAT floats[4]);
	static void APIENTRY cs_set_unordered_access_views(D3D10DDI_HDEVICE device, UINT offset, UINT num_views,
		const D3D11DDI_HUNORDEREDACCESSVIEW* unordered_access_view, const UINT* uav_initial_counts);
	static void APIENTRY dispatch(D3D10DDI_HDEVICE device, UINT thread_group_count_x,
		UINT thread_group_count_y, UINT thread_group_count_z);
	static void APIENTRY dispatch_indirect(D3D10DDI_HDEVICE device,
		D3D10DDI_HRESOURCE buffer_for_args, UINT aligned_byte_offset_for_args);
	static void APIENTRY set_resource_min_lod(D3D10DDI_HDEVICE device, D3D10DDI_HRESOURCE resource,
		FLOAT min_lod);
	static void APIENTRY copy_structure_count(D3D10DDI_HDEVICE device, D3D10DDI_HRESOURCE dst_buffer,
		UINT dst_aligned_byte_offset, D3D11DDI_HUNORDEREDACCESSVIEW src_view);
	static void APIENTRY recycle_command_list(D3D10DDI_HDEVICE device, D3D11DDI_HCOMMANDLIST command_list);
	static HRESULT APIENTRY recycle_create_command_list(D3D10DDI_HDEVICE device,
		const D3D11DDIARG_CREATECOMMANDLIST* create_command_list, D3D11DDI_HCOMMANDLIST command_list,
		D3D11DDI_HRTCOMMANDLIST rt_command_list);
	static HRESULT APIENTRY recycle_create_deferred_context(D3D10DDI_HDEVICE device,
		const D3D11DDIARG_CREATEDEFERREDCONTEXT* create_deferred_context);
	static void APIENTRY recycle_destroy_command_list(D3D10DDI_HDEVICE device, D3D11DDI_HCOMMANDLIST command_list);

	static HRESULT APIENTRY present_dxgi(DXGI_DDI_ARG_PRESENT* present_data);
	static HRESULT APIENTRY get_gamma_caps_dxgi(DXGI_DDI_ARG_GET_GAMMA_CONTROL_CAPS* gamma_data);
	static HRESULT APIENTRY set_display_mode_dxgi(DXGI_DDI_ARG_SETDISPLAYMODE* display_mode_data);
	static HRESULT APIENTRY set_resource_priority_dxgi(DXGI_DDI_ARG_SETRESOURCEPRIORITY* priority_data);
	static HRESULT APIENTRY query_resource_residency_dxgi(DXGI_DDI_ARG_QUERYRESOURCERESIDENCY* residency_data);
	static HRESULT APIENTRY rotate_resource_identities_dxgi(DXGI_DDI_ARG_ROTATE_RESOURCE_IDENTITIES* rotate_data);
	static HRESULT APIENTRY blt_dxgi(DXGI_DDI_ARG_BLT* blt_data);
	static HRESULT APIENTRY resolve_shared_resource_dxgi(DXGI_DDI_ARG_RESOLVESHAREDRESOURCE* resource_data);

	static HRESULT APIENTRY blt1_dxgi(DXGI_DDI_ARG_BLT1* blt1_data);
	static HRESULT APIENTRY offer_resources(DXGI_DDI_ARG_OFFERRESOURCES* resources);
	static HRESULT APIENTRY reclaim_resources(DXGI_DDI_ARG_RECLAIMRESOURCES* resources);
	static HRESULT APIENTRY get_multi_plane_overlay_caps(DXGI_DDI_ARG_GETMULTIPLANEOVERLAYCAPS* caps);
	static HRESULT APIENTRY check_multi_plane_overlay_support(DXGI_DDI_ARG_CHECKMULTIPLANEOVERLAYSUPPORT* support);
	static HRESULT APIENTRY present_multi_plane_overlay(DXGI_DDI_ARG_PRESENTMULTIPLANEOVERLAY* present_dxgi);

	// TODO

private:
	umd_adapter* adapter_;

	D3D11DDI_DEVICEFUNCS* d3d11_device_funcs_;

	D3D10DDI_HRTDEVICE d3d_rt_device_;
	D3D10DDI_HRTCORELAYER d3d_rt_core_layer_;

	const D3DDDI_DEVICECALLBACKS* d3d_device_cb_;
	const D3D10DDI_CORELAYER_DEVICECALLBACKS* d3d_core_layer_device_cb_;
	const DXGI_DDI_BASE_CALLBACKS* dxgi_base_cb_;

	HANDLE d3d_cb_context_;

	D3DKMT_CREATEDCFROMMEMORY dc_from_mem_;

	salviar::renderer_ptr sa_renderer_;
	salviar::surface_ptr resolved_surf_;
};
