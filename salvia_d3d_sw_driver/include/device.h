#pragma once

class umd_device
{
public:
    umd_device(umd_adapter* adapter, const D3D10DDIARG_CREATEDEVICE* args);
    ~umd_device();

    void destroy();

    void set_d3d_error(HRESULT hr);

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

    // TODO

private:
    umd_adapter* adapter_;

    D3D11DDI_DEVICEFUNCS* d3d11_device_funcs_;

    D3D10DDI_HRTDEVICE d3d_rt_device_;
    D3D10DDI_HRTCORELAYER d3d_rt_core_layer_;

    const D3DDDI_DEVICECALLBACKS* d3d_device_cb_;
    const D3D10DDI_CORELAYER_DEVICECALLBACKS* d3d_core_layer_device_cb_;

    HANDLE d3d_cb_context_;
};
