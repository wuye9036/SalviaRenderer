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
    static void APIENTRY resource_map(D3D10DDI_HDEVICE device, D3D10DDI_HRESOURCE resource,
        UINT subresource, D3D10_DDI_MAP ddi_map, UINT flags,
        D3D10DDI_MAPPED_SUBRESOURCE* mapped_subresource);
    static void APIENTRY resource_unmap(D3D10DDI_HDEVICE device, D3D10DDI_HRESOURCE resource,
        UINT subresource);
    static void APIENTRY ps_set_constant_buffers(D3D10DDI_HDEVICE device, UINT start_buffer,
        UINT num_buffers, const D3D10DDI_HRESOURCE* buffers);
    static void APIENTRY ia_set_input_layout(D3D10DDI_HDEVICE device,
        D3D10DDI_HELEMENTLAYOUT input_layout);
    static void APIENTRY ia_set_vertex_buffers(D3D10DDI_HDEVICE device, UINT start_buffer,
        UINT num_buffers, const D3D10DDI_HRESOURCE* buffers, const UINT* strides, const UINT* offsets);
    static void APIENTRY ia_set_index_buffer(D3D10DDI_HDEVICE device, D3D10DDI_HRESOURCE buffer,
        DXGI_FORMAT format, UINT offset);

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
