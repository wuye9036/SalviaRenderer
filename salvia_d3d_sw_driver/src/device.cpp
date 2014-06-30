#define INITGUID
#include <salvia_d3d_sw_driver/include/common.h>

#include <eflib/include/platform/typedefs.h>

#include <salvia_d3d_sw_driver/include/adapter.h>
#include <salvia_d3d_sw_driver/include/device.h>

umd_device::umd_device(umd_adapter* adapter, const D3D10DDIARG_CREATEDEVICE* args)
    : adapter_(adapter), 
        d3d_rt_device_(args->hRTDevice), d3d_rt_core_layer_(args->hRTCoreLayer),
        d3d_device_cb_(args->pKTCallbacks), d3d_core_layer_device_cb_(args->pUMCallbacks),
        d3d_cb_context_(nullptr)
{
    *args->ppfnRetrieveSubObject = nullptr;

    switch (args->Interface)
    {
    case D3D11_0_DDI_INTERFACE_VERSION:
    case D3D11_0_vista_DDI_INTERFACE_VERSION:
    case D3D11_0_7_DDI_INTERFACE_VERSION:
        d3d11_device_funcs_ = args->p11DeviceFuncs;

        // HIGH-FREQUENCY
        d3d11_device_funcs_->pfnDefaultConstantBufferUpdateSubresourceUP = default_constant_buffer_update_subresource_up;
        d3d11_device_funcs_->pfnVsSetConstantBuffers = vs_set_constant_buffers;
        d3d11_device_funcs_->pfnPsSetShaderResources = ps_set_shader_resources;
        d3d11_device_funcs_->pfnPsSetShader = ps_set_shader;
        d3d11_device_funcs_->pfnPsSetSamplers = ps_set_samplers;
        d3d11_device_funcs_->pfnVsSetShader = vs_set_shader;
        d3d11_device_funcs_->pfnDrawIndexed = draw_indexed;
        d3d11_device_funcs_->pfnDraw = draw;
        d3d11_device_funcs_->pfnDynamicIABufferMapNoOverwrite = dynamic_ia_buffer_map_no_overwrite;
        d3d11_device_funcs_->pfnDynamicIABufferUnmap = dynamic_ia_buffer_unmap;
        d3d11_device_funcs_->pfnDynamicConstantBufferMapDiscard = dynamic_constant_buffer_map_discard;
        d3d11_device_funcs_->pfnDynamicIABufferMapDiscard = dynamic_ia_buffer_map_discard;
        d3d11_device_funcs_->pfnDynamicConstantBufferUnmap = dynamic_constant_buffer_unmap;
        d3d11_device_funcs_->pfnPsSetConstantBuffers = ps_set_constant_buffers;
        d3d11_device_funcs_->pfnIaSetInputLayout = ia_set_input_layout;
        d3d11_device_funcs_->pfnIaSetVertexBuffers = ia_set_vertex_buffers;
        d3d11_device_funcs_->pfnIaSetIndexBuffer = ia_set_index_buffer;

        // MIDDLE-FREQUENCY
        d3d11_device_funcs_->pfnDrawIndexedInstanced = draw_indexed_instanced;
        d3d11_device_funcs_->pfnDrawInstanced = draw_instanced;
        d3d11_device_funcs_->pfnDynamicResourceMapDiscard = dynamic_resource_map_discard;
        d3d11_device_funcs_->pfnDynamicResourceUnmap = dynamic_resource_unmap;
        d3d11_device_funcs_->pfnGsSetConstantBuffers = gs_set_constant_buffers;
        d3d11_device_funcs_->pfnGsSetShader = gs_set_shader;
        d3d11_device_funcs_->pfnIaSetTopology = ia_set_topology;
        d3d11_device_funcs_->pfnStagingResourceMap = staging_resource_map;
        d3d11_device_funcs_->pfnStagingResourceUnmap = staging_resource_unmap;
        d3d11_device_funcs_->pfnVsSetShaderResources = vs_set_shader_resources;
        d3d11_device_funcs_->pfnVsSetSamplers = vs_set_samplers;
        d3d11_device_funcs_->pfnGsSetShaderResources = gs_set_shader_resources;
        d3d11_device_funcs_->pfnGsSetSamplers = gs_set_samplers;
        d3d11_device_funcs_->pfnSetRenderTargets = set_render_targets;
        d3d11_device_funcs_->pfnShaderResourceViewReadAfterWriteHazard = shader_resource_view_read_after_write_hazard;
        d3d11_device_funcs_->pfnResourceReadAfterWriteHazard = resource_read_after_write_hazard;
        d3d11_device_funcs_->pfnSetBlendState = set_blend_state;
        d3d11_device_funcs_->pfnSetDepthStencilState = set_depth_stencil_state;
        d3d11_device_funcs_->pfnSetRasterizerState = set_rasterizer_state;
        d3d11_device_funcs_->pfnQueryEnd = query_end;
        d3d11_device_funcs_->pfnQueryBegin = query_begin;
        d3d11_device_funcs_->pfnResourceCopyRegion = resource_copy_region;
        d3d11_device_funcs_->pfnResourceUpdateSubresourceUP = resource_update_subresource_up;
        d3d11_device_funcs_->pfnSoSetTargets = so_set_targets;
        d3d11_device_funcs_->pfnDrawAuto = draw_auto;
        d3d11_device_funcs_->pfnSetViewports = set_viewports;
        d3d11_device_funcs_->pfnSetScissorRects = set_scissor_rects;
        d3d11_device_funcs_->pfnClearRenderTargetView = clear_render_target_view;
        d3d11_device_funcs_->pfnClearDepthStencilView = clear_depth_stencil_view;
        d3d11_device_funcs_->pfnSetPredication = set_predication;
        d3d11_device_funcs_->pfnQueryGetData = query_get_data;
        d3d11_device_funcs_->pfnFlush = flush;
        d3d11_device_funcs_->pfnGenMips = gen_mips;
        d3d11_device_funcs_->pfnResourceCopy = resource_copy;
        d3d11_device_funcs_->pfnResourceResolveSubresource = resource_resolve_subresource;

        // Infrequent paths
        d3d11_device_funcs_->pfnResourceMap = resource_map;
        d3d11_device_funcs_->pfnResourceUnmap = resource_unmap;
        d3d11_device_funcs_->pfnResourceIsStagingBusy = resource_is_staging_busy;
        d3d11_device_funcs_->pfnRelocateDeviceFuncs = relocate_device_funcs;
        d3d11_device_funcs_->pfnCalcPrivateResourceSize = calc_private_resource_size;
        d3d11_device_funcs_->pfnCalcPrivateOpenedResourceSize = calc_private_opened_resource_size;
        d3d11_device_funcs_->pfnCreateResource = create_resource;
        d3d11_device_funcs_->pfnOpenResource = open_resource;
        d3d11_device_funcs_->pfnDestroyResource = destroy_resource;
        d3d11_device_funcs_->pfnCalcPrivateShaderResourceViewSize = calc_private_shader_resource_view_size;
        d3d11_device_funcs_->pfnCreateShaderResourceView = create_shader_resource_view;
        d3d11_device_funcs_->pfnDestroyShaderResourceView = destroy_shader_resource_view;
        d3d11_device_funcs_->pfnCalcPrivateRenderTargetViewSize = calc_private_render_target_view_size;
        d3d11_device_funcs_->pfnCreateRenderTargetView = create_render_target_view;
        d3d11_device_funcs_->pfnDestroyRenderTargetView = destroy_render_target_view;
        d3d11_device_funcs_->pfnCalcPrivateDepthStencilViewSize = calc_private_depth_stencil_view_size;
        d3d11_device_funcs_->pfnCreateDepthStencilView = create_depth_stencil_view;
        d3d11_device_funcs_->pfnDestroyDepthStencilView = destroy_depth_stencil_view;
        d3d11_device_funcs_->pfnCalcPrivateElementLayoutSize = calc_private_element_layout_size;
        d3d11_device_funcs_->pfnCreateElementLayout = create_element_layout;
        d3d11_device_funcs_->pfnDestroyElementLayout = destroy_element_layout;
        d3d11_device_funcs_->pfnCalcPrivateBlendStateSize = calc_private_blend_state_size;
        d3d11_device_funcs_->pfnCreateBlendState = create_blend_state;
        d3d11_device_funcs_->pfnDestroyBlendState = destroy_blend_state;
        d3d11_device_funcs_->pfnCalcPrivateDepthStencilStateSize = calc_private_depth_stencil_state_size;
        d3d11_device_funcs_->pfnCreateDepthStencilState = create_depth_stencil_state;
        d3d11_device_funcs_->pfnDestroyDepthStencilState = destroy_depth_stencil_state;
        d3d11_device_funcs_->pfnCalcPrivateRasterizerStateSize = calc_private_rasterizer_stateSize;
        d3d11_device_funcs_->pfnCreateRasterizerState = create_rasterizer_state;
        d3d11_device_funcs_->pfnDestroyRasterizerState = destroy_rasterizer_state;
        d3d11_device_funcs_->pfnCalcPrivateShaderSize = calc_private_shader_size;
        d3d11_device_funcs_->pfnCreateVertexShader = create_vertex_shader;
        d3d11_device_funcs_->pfnCreateGeometryShader = create_geometry_shader;
        d3d11_device_funcs_->pfnCreatePixelShader = create_pixel_shader;
        d3d11_device_funcs_->pfnCalcPrivateGeometryShaderWithStreamOutput = calc_private_geometry_shader_with_stream_output;
        d3d11_device_funcs_->pfnCreateGeometryShaderWithStreamOutput = create_geometry_shader_with_stream_output;
        d3d11_device_funcs_->pfnDestroyShader = destroy_shader;
        d3d11_device_funcs_->pfnCalcPrivateSamplerSize = calc_private_sampler_size;
        d3d11_device_funcs_->pfnCreateSampler = create_sampler;
        d3d11_device_funcs_->pfnDestroySampler = destroy_sampler;
        d3d11_device_funcs_->pfnCalcPrivateQuerySize = calc_private_query_size;
        d3d11_device_funcs_->pfnCreateQuery = create_query;
        d3d11_device_funcs_->pfnDestroyQuery = destroy_query;
        d3d11_device_funcs_->pfnCheckFormatSupport = check_format_support;
        d3d11_device_funcs_->pfnCheckMultisampleQualityLevels = check_multisample_quality_levels;
        d3d11_device_funcs_->pfnCheckCounterInfo = check_counter_info;
        d3d11_device_funcs_->pfnCheckCounter = check_counter;
        d3d11_device_funcs_->pfnDestroyDevice = destroy_device;
        d3d11_device_funcs_->pfnSetTextFilterSize = set_text_filter_size;

        // Additional 10.1 entries
        d3d11_device_funcs_->pfnResourceConvert = resource_convert;
        d3d11_device_funcs_->pfnResourceConvertRegion = resource_convert_region;

        // Additional 11.0 entries
        d3d11_device_funcs_->pfnDrawIndexedInstancedIndirect = draw_indexed_instanced_indirect;
        d3d11_device_funcs_->pfnDrawInstancedIndirect = draw_instanced_indirect;
        d3d11_device_funcs_->pfnCommandListExecute = command_list_execute;
        d3d11_device_funcs_->pfnHsSetShaderResources = hs_set_shader_resources;
        d3d11_device_funcs_->pfnHsSetShader = hs_set_shader;
        d3d11_device_funcs_->pfnHsSetSamplers = hs_set_samplers;
        d3d11_device_funcs_->pfnHsSetConstantBuffers = hs_set_constant_buffers;
        d3d11_device_funcs_->pfnDsSetShaderResources = ds_set_shader_resources;
        d3d11_device_funcs_->pfnDsSetShader = ds_set_shader;
        d3d11_device_funcs_->pfnDsSetSamplers = ds_set_samplers;
        d3d11_device_funcs_->pfnDsSetConstantBuffers = ds_set_constant_buffers;
        d3d11_device_funcs_->pfnCreateHullShader = create_hull_shader;
        d3d11_device_funcs_->pfnCreateDomainShader = create_domain_shader;
        d3d11_device_funcs_->pfnCheckDeferredContextHandleSizes = check_deferred_context_handle_sizes;
        d3d11_device_funcs_->pfnCalcDeferredContextHandleSize = calc_deferred_context_handle_size;
        d3d11_device_funcs_->pfnCalcPrivateDeferredContextSize = calc_private_deferred_context_size;
        d3d11_device_funcs_->pfnCreateDeferredContext = create_deferred_context;
        d3d11_device_funcs_->pfnAbandonCommandList = abandon_command_list;
        d3d11_device_funcs_->pfnCalcPrivateCommandListSize = calc_private_command_list_size;
        d3d11_device_funcs_->pfnCreateCommandList = create_command_list;
        d3d11_device_funcs_->pfnDestroyCommandList = destroy_command_list;
        d3d11_device_funcs_->pfnCalcPrivateTessellationShaderSize = calc_private_tessellation_shader_size;
        d3d11_device_funcs_->pfnPsSetShaderWithIfaces = ps_set_shader_with_ifaces;
        d3d11_device_funcs_->pfnVsSetShaderWithIfaces = vs_set_shader_with_ifaces;
        d3d11_device_funcs_->pfnGsSetShaderWithIfaces = gs_set_shader_with_ifaces;
        d3d11_device_funcs_->pfnHsSetShaderWithIfaces = hs_set_shader_with_ifaces;
        d3d11_device_funcs_->pfnDsSetShaderWithIfaces = ds_set_shader_with_ifaces;
        d3d11_device_funcs_->pfnCsSetShaderWithIfaces = cs_set_shader_with_ifaces;
        d3d11_device_funcs_->pfnCreateComputeShader = create_compute_shader;
        d3d11_device_funcs_->pfnCsSetShader = cs_set_shader;
        d3d11_device_funcs_->pfnCsSetShaderResources = cs_set_shader_resources;
        d3d11_device_funcs_->pfnCsSetSamplers = cs_set_samplers;
        d3d11_device_funcs_->pfnCsSetConstantBuffers = cs_set_constant_buffers;
        d3d11_device_funcs_->pfnCalcPrivateUnorderedAccessViewSize = calc_private_unordered_access_view_size;
        d3d11_device_funcs_->pfnCreateUnorderedAccessView = create_unordered_access_view;
        d3d11_device_funcs_->pfnDestroyUnorderedAccessView = destroy_unordered_access_view;
        d3d11_device_funcs_->pfnClearUnorderedAccessViewUint = clear_unordered_access_view_uint;
        d3d11_device_funcs_->pfnClearUnorderedAccessViewFloat = clear_unordered_access_view_float;
        d3d11_device_funcs_->pfnCsSetUnorderedAccessViews = cs_set_unordered_access_views;
        d3d11_device_funcs_->pfnDispatch = dispatch;
        d3d11_device_funcs_->pfnDispatchIndirect = dispatch_indirect;
        d3d11_device_funcs_->pfnSetResourceMinLOD = set_resource_min_lod;
        d3d11_device_funcs_->pfnCopyStructureCount = copy_structure_count;

        // D3D11_0_*DDI_BUILD_VERSION == 2
        d3d11_device_funcs_->pfnRecycleCommandList = recycle_command_list;
        d3d11_device_funcs_->pfnRecycleCreateCommandList = recycle_create_command_list;
        d3d11_device_funcs_->pfnRecycleCreateDeferredContext = recycle_create_deferred_context;
        d3d11_device_funcs_->pfnRecycleDestroyCommandList = recycle_destroy_command_list;
        break;

#if D3D11DDI_MINOR_HEADER_VERSION >= 3
    case D3D11_1_DDI_INTERFACE_VERSION:
        // TODO: D3D 11.1?
        break;
#endif

    default:
        break;
    }

#if (D3D_UMD_INTERFACE_VERSION >= D3D_UMD_INTERFACE_VERSION_WIN8)
    DXGI1_2_DDI_BASE_FUNCTIONS* dxgi_ddi_funcs = args->DXGIBaseDDI.pDXGIDDIBaseFunctions3;
#else
    DXGI1_1_DDI_BASE_FUNCTIONS* dxgi_ddi_funcs = args->DXGIBaseDDI.pDXGIDDIBaseFunctions2;
#endif
    dxgi_ddi_funcs->pfnPresent = present_dxgi;
    dxgi_ddi_funcs->pfnGetGammaCaps = get_gamma_caps_dxgi;
    dxgi_ddi_funcs->pfnSetDisplayMode = set_display_mode_dxgi;
    dxgi_ddi_funcs->pfnSetResourcePriority = set_resource_priority_dxgi;
    dxgi_ddi_funcs->pfnQueryResourceResidency = query_resource_residency_dxgi;
    dxgi_ddi_funcs->pfnRotateResourceIdentities = rotate_resource_identities_dxgi;
    dxgi_ddi_funcs->pfnBlt = blt_dxgi;
    dxgi_ddi_funcs->pfnResolveSharedResource = resolve_shared_resource_dxgi;
#if (D3D_UMD_INTERFACE_VERSION >= D3D_UMD_INTERFACE_VERSION_WIN8)
    dxgi_ddi_funcs->pfnBlt1 = blt1_dxgi;
    dxgi_ddi_funcs->pfnOfferResources = offer_resources;
    dxgi_ddi_funcs->pfnReclaimResources = reclaim_resources;
    dxgi_ddi_funcs->pfnGetMultiPlaneOverlayCaps = get_multi_plane_overlay_caps;
    dxgi_ddi_funcs->pfnGetMultiPlaneOverlayFilterRange = nullptr;
    dxgi_ddi_funcs->pfnCheckMultiPlaneOverlaySupport = check_multi_plane_overlay_support;
    dxgi_ddi_funcs->pfnPresentMultiPlaneOverlay = present_multi_plane_overlay;
#endif

    D3DDDICB_CREATECONTEXT d3d_cb_create_context;
    memset(&d3d_cb_create_context, 0, sizeof(d3d_cb_create_context));
    HRESULT hr = d3d_device_cb_->pfnCreateContextCb(d3d_rt_device_.handle, &d3d_cb_create_context);
    if (SUCCEEDED(hr))
    {
        d3d_cb_context_ = d3d_cb_create_context.hContext;

        // TODO: Create SALVIA device here
    }

    if (FAILED(hr))
    {
        this->set_d3d_error(hr);
        this->destroy();
    }
}

umd_device::~umd_device()
{
    this->destroy();
}

void umd_device::destroy()
{
    if (d3d_cb_context_ != nullptr)
    {
        D3DDDICB_DESTROYCONTEXT d3d_cb_destroy_context;
        d3d_cb_destroy_context.hContext = d3d_cb_context_;
        d3d_device_cb_->pfnDestroyContextCb(d3d_rt_device_.handle, &d3d_cb_destroy_context);
        d3d_cb_context_ = nullptr;
    }
}

void umd_device::set_d3d_error(HRESULT hr)
{
    (*d3d_core_layer_device_cb_->pfnSetErrorCb)(d3d_rt_core_layer_, hr);
}

void umd_device::default_constant_buffer_update_subresource_up(D3D10DDI_HDEVICE device,
        D3D10DDI_HRESOURCE dst_resource, UINT dst_subresource, const D3D10_DDI_BOX* dst_box,
        const void* sys_mem_up, UINT row_pitch, UINT depth_pitch)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(dst_resource);
    UNREFERENCED_PARAMETER(dst_subresource);
    UNREFERENCED_PARAMETER(dst_box);
    UNREFERENCED_PARAMETER(sys_mem_up);
    UNREFERENCED_PARAMETER(row_pitch);
    UNREFERENCED_PARAMETER(depth_pitch);
}

void umd_device::vs_set_constant_buffers(D3D10DDI_HDEVICE device, UINT start_buffer,
        UINT num_buffers, const D3D10DDI_HRESOURCE* buffers)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(start_buffer);
    UNREFERENCED_PARAMETER(num_buffers);
    UNREFERENCED_PARAMETER(buffers);
}

void umd_device::ps_set_shader_resources(D3D10DDI_HDEVICE device, UINT offset,
        UINT num_views, const D3D10DDI_HSHADERRESOURCEVIEW* srvs)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(offset);
    UNREFERENCED_PARAMETER(num_views);
    UNREFERENCED_PARAMETER(srvs);
}

void umd_device::ps_set_shader(D3D10DDI_HDEVICE device, D3D10DDI_HSHADER shader)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(shader);
}

void umd_device::ps_set_samplers(D3D10DDI_HDEVICE device, UINT offset,
        UINT num_samplers, const D3D10DDI_HSAMPLER* samplers)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(offset);
    UNREFERENCED_PARAMETER(num_samplers);
    UNREFERENCED_PARAMETER(samplers);
}

void umd_device::vs_set_shader(D3D10DDI_HDEVICE device, D3D10DDI_HSHADER shader)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(shader);
}

void umd_device::draw_indexed(D3D10DDI_HDEVICE device, UINT index_count,
        UINT start_index_location, INT base_vertex_location)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(index_count);
    UNREFERENCED_PARAMETER(start_index_location);
    UNREFERENCED_PARAMETER(base_vertex_location);
}

void umd_device::draw(D3D10DDI_HDEVICE device, UINT vertex_count,
        UINT start_vertex_location)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(vertex_count);
    UNREFERENCED_PARAMETER(start_vertex_location);
}

void umd_device::dynamic_ia_buffer_map_no_overwrite(D3D10DDI_HDEVICE device, D3D10DDI_HRESOURCE resource,
        UINT subresource, D3D10_DDI_MAP ddi_map, UINT flags,
        D3D10DDI_MAPPED_SUBRESOURCE* mapped_subresource)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(resource);
    UNREFERENCED_PARAMETER(subresource);
    UNREFERENCED_PARAMETER(ddi_map);
    UNREFERENCED_PARAMETER(flags);
    UNREFERENCED_PARAMETER(mapped_subresource);
}

void umd_device::dynamic_ia_buffer_unmap(D3D10DDI_HDEVICE device, D3D10DDI_HRESOURCE resource,
        UINT subresource)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(resource);
    UNREFERENCED_PARAMETER(subresource);
}

void umd_device::dynamic_constant_buffer_map_discard(D3D10DDI_HDEVICE device, D3D10DDI_HRESOURCE resource,
        UINT subresource, D3D10_DDI_MAP ddi_map, UINT flags,
        D3D10DDI_MAPPED_SUBRESOURCE* mapped_subresource)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(resource);
    UNREFERENCED_PARAMETER(subresource);
    UNREFERENCED_PARAMETER(ddi_map);
    UNREFERENCED_PARAMETER(flags);
    UNREFERENCED_PARAMETER(mapped_subresource);
}

void umd_device::dynamic_ia_buffer_map_discard(D3D10DDI_HDEVICE device, D3D10DDI_HRESOURCE resource,
        UINT subresource, D3D10_DDI_MAP ddi_map, UINT flags,
        D3D10DDI_MAPPED_SUBRESOURCE* mapped_subresource)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(resource);
    UNREFERENCED_PARAMETER(subresource);
    UNREFERENCED_PARAMETER(ddi_map);
    UNREFERENCED_PARAMETER(flags);
    UNREFERENCED_PARAMETER(mapped_subresource);
}

void umd_device::dynamic_constant_buffer_unmap(D3D10DDI_HDEVICE device, D3D10DDI_HRESOURCE resource,
        UINT subresource)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(resource);
    UNREFERENCED_PARAMETER(subresource);
}

void umd_device::ps_set_constant_buffers(D3D10DDI_HDEVICE device, UINT start_buffer,
        UINT num_buffers, const D3D10DDI_HRESOURCE* buffers)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(start_buffer);
    UNREFERENCED_PARAMETER(num_buffers);
    UNREFERENCED_PARAMETER(buffers);
}

void umd_device::ia_set_input_layout(D3D10DDI_HDEVICE device,
        D3D10DDI_HELEMENTLAYOUT input_layout)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(input_layout);
}

void umd_device::ia_set_vertex_buffers(D3D10DDI_HDEVICE device, UINT start_buffer,
        UINT num_buffers, const D3D10DDI_HRESOURCE* buffers, const UINT* strides, const UINT* offsets)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(start_buffer);
    UNREFERENCED_PARAMETER(num_buffers);
    UNREFERENCED_PARAMETER(buffers);
    UNREFERENCED_PARAMETER(strides);
    UNREFERENCED_PARAMETER(offsets);
}

void umd_device::ia_set_index_buffer(D3D10DDI_HDEVICE device, D3D10DDI_HRESOURCE buffer,
        DXGI_FORMAT format, UINT offset)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(buffer);
    UNREFERENCED_PARAMETER(format);
    UNREFERENCED_PARAMETER(offset);
}

void umd_device::draw_indexed_instanced(D3D10DDI_HDEVICE device, UINT index_count_per_instance,
        UINT instance_count, UINT start_index_location, INT base_vertex_location, UINT start_instance_location)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(index_count_per_instance);
    UNREFERENCED_PARAMETER(instance_count);
    UNREFERENCED_PARAMETER(start_index_location);
    UNREFERENCED_PARAMETER(base_vertex_location);
    UNREFERENCED_PARAMETER(start_instance_location);
}

void umd_device::draw_instanced(D3D10DDI_HDEVICE device, UINT vertex_count_per_instance,
        UINT instance_count, UINT start_vertex_location, UINT start_instance_location)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(vertex_count_per_instance);
    UNREFERENCED_PARAMETER(instance_count);
    UNREFERENCED_PARAMETER(start_vertex_location);
    UNREFERENCED_PARAMETER(start_instance_location);
}

void umd_device::dynamic_resource_map_discard(D3D10DDI_HDEVICE device, D3D10DDI_HRESOURCE resource,
        UINT subresource, D3D10_DDI_MAP ddi_map, UINT flags,
        D3D10DDI_MAPPED_SUBRESOURCE* mapped_subresource)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(resource);
    UNREFERENCED_PARAMETER(subresource);
    UNREFERENCED_PARAMETER(ddi_map);
    UNREFERENCED_PARAMETER(flags);
    UNREFERENCED_PARAMETER(mapped_subresource);
}

void umd_device::dynamic_resource_unmap(D3D10DDI_HDEVICE device, D3D10DDI_HRESOURCE resource,
        UINT subresource)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(resource);
    UNREFERENCED_PARAMETER(subresource);
}

void umd_device::gs_set_constant_buffers(D3D10DDI_HDEVICE device, UINT start_buffer,
        UINT num_buffers, const D3D10DDI_HRESOURCE* buffers)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(start_buffer);
    UNREFERENCED_PARAMETER(num_buffers);
    UNREFERENCED_PARAMETER(buffers);
}

void umd_device::gs_set_shader(D3D10DDI_HDEVICE device, D3D10DDI_HSHADER shader)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(shader);
}

void umd_device::ia_set_topology(D3D10DDI_HDEVICE device, D3D10_DDI_PRIMITIVE_TOPOLOGY primitive_topology)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(primitive_topology);
}

void umd_device::staging_resource_map(D3D10DDI_HDEVICE device, D3D10DDI_HRESOURCE resource,
        UINT subresource, D3D10_DDI_MAP ddi_map, UINT flags,
        D3D10DDI_MAPPED_SUBRESOURCE* mapped_subresource)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(resource);
    UNREFERENCED_PARAMETER(subresource);
    UNREFERENCED_PARAMETER(ddi_map);
    UNREFERENCED_PARAMETER(flags);
    UNREFERENCED_PARAMETER(mapped_subresource);
}

void umd_device::staging_resource_unmap(D3D10DDI_HDEVICE device, D3D10DDI_HRESOURCE resource,
        UINT subresource)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(resource);
    UNREFERENCED_PARAMETER(subresource);
}

void umd_device::vs_set_shader_resources(D3D10DDI_HDEVICE device, UINT offset, UINT num_views,
        const D3D10DDI_HSHADERRESOURCEVIEW* shader_resource_views)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(offset);
    UNREFERENCED_PARAMETER(num_views);
    UNREFERENCED_PARAMETER(shader_resource_views);
}

void umd_device::vs_set_samplers(D3D10DDI_HDEVICE device, UINT offset, UINT num_samplers,
        const D3D10DDI_HSAMPLER* samplers)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(offset);
    UNREFERENCED_PARAMETER(num_samplers);
    UNREFERENCED_PARAMETER(samplers);
}

void umd_device::gs_set_shader_resources(D3D10DDI_HDEVICE device, UINT offset, UINT num_views,
        const D3D10DDI_HSHADERRESOURCEVIEW* shader_resource_views)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(offset);
    UNREFERENCED_PARAMETER(num_views);
    UNREFERENCED_PARAMETER(shader_resource_views);
}

void umd_device::gs_set_samplers(D3D10DDI_HDEVICE device, UINT offset, UINT num_samplers,
        const D3D10DDI_HSAMPLER* samplers)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(offset);
    UNREFERENCED_PARAMETER(num_samplers);
    UNREFERENCED_PARAMETER(samplers);
}

void umd_device::set_render_targets(D3D10DDI_HDEVICE device,
        const D3D10DDI_HRENDERTARGETVIEW* render_target_view, UINT num_rtvs, UINT rtv_number_to_unbind,
        D3D10DDI_HDEPTHSTENCILVIEW depth_stencil_view, const D3D11DDI_HUNORDEREDACCESSVIEW* unordered_access_view,
        const UINT* uav_initial_counts, UINT uav_index, UINT num_uavs, UINT uav_first_to_set, UINT uav_number_updated)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(render_target_view);
    UNREFERENCED_PARAMETER(num_rtvs);
    UNREFERENCED_PARAMETER(rtv_number_to_unbind);
    UNREFERENCED_PARAMETER(depth_stencil_view);
    UNREFERENCED_PARAMETER(unordered_access_view);
    UNREFERENCED_PARAMETER(uav_initial_counts);
    UNREFERENCED_PARAMETER(uav_index);
    UNREFERENCED_PARAMETER(num_uavs);
    UNREFERENCED_PARAMETER(uav_first_to_set);
    UNREFERENCED_PARAMETER(uav_number_updated);
}

void APIENTRY umd_device::shader_resource_view_read_after_write_hazard(D3D10DDI_HDEVICE device,
        D3D10DDI_HSHADERRESOURCEVIEW shader_resource_view, D3D10DDI_HRESOURCE resource)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(shader_resource_view);
    UNREFERENCED_PARAMETER(resource);
}

void APIENTRY umd_device::resource_read_after_write_hazard(D3D10DDI_HDEVICE device, D3D10DDI_HRESOURCE resource)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(resource);
}

void APIENTRY umd_device::set_blend_state(D3D10DDI_HDEVICE device, D3D10DDI_HBLENDSTATE state,
        const FLOAT blend_factor[4], UINT sample_mask)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(state);
    UNREFERENCED_PARAMETER(blend_factor);
    UNREFERENCED_PARAMETER(sample_mask);
}

void APIENTRY umd_device::set_depth_stencil_state(D3D10DDI_HDEVICE device, D3D10DDI_HDEPTHSTENCILSTATE state,
        UINT stencil_ref)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(state);    
    UNREFERENCED_PARAMETER(stencil_ref);
}

void APIENTRY umd_device::set_rasterizer_state(D3D10DDI_HDEVICE device, D3D10DDI_HRASTERIZERSTATE state)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(state);
}

void APIENTRY umd_device::query_end(D3D10DDI_HDEVICE device, D3D10DDI_HQUERY query)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(query);
}

void APIENTRY umd_device::query_begin(D3D10DDI_HDEVICE device, D3D10DDI_HQUERY query)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(query);
}

void APIENTRY umd_device::resource_copy_region(D3D10DDI_HDEVICE device, D3D10DDI_HRESOURCE dst_resource, UINT dst_subresource,
        UINT dst_x, UINT dst_y, UINT dst_z, D3D10DDI_HRESOURCE src_resource, UINT src_subresource, const D3D10_DDI_BOX* src_box)       
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(dst_resource);
    UNREFERENCED_PARAMETER(dst_subresource);
    UNREFERENCED_PARAMETER(dst_x);
    UNREFERENCED_PARAMETER(dst_y);
    UNREFERENCED_PARAMETER(dst_z);
    UNREFERENCED_PARAMETER(src_resource);
    UNREFERENCED_PARAMETER(src_subresource);
    UNREFERENCED_PARAMETER(src_box);
}
void APIENTRY umd_device::resource_update_subresource_up(D3D10DDI_HDEVICE device, D3D10DDI_HRESOURCE dst_resource,
        UINT dst_subresource, const D3D10_DDI_BOX* dst_box, const VOID* sys_mem_up, UINT row_pitch, UINT depth_pitch)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(dst_resource);
    UNREFERENCED_PARAMETER(dst_subresource);
    UNREFERENCED_PARAMETER(dst_box);
    UNREFERENCED_PARAMETER(sys_mem_up);
    UNREFERENCED_PARAMETER(row_pitch);
    UNREFERENCED_PARAMETER(depth_pitch);
}

void APIENTRY umd_device::so_set_targets(D3D10DDI_HDEVICE device, UINT so_targets, UINT clear_targets,
        const D3D10DDI_HRESOURCE* resource, const UINT* offsets)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(so_targets);
    UNREFERENCED_PARAMETER(clear_targets);
    UNREFERENCED_PARAMETER(resource);
    UNREFERENCED_PARAMETER(offsets);
}

void APIENTRY umd_device::draw_auto(D3D10DDI_HDEVICE device)
{
    UNREFERENCED_PARAMETER(device);
}

void APIENTRY umd_device::set_viewports(D3D10DDI_HDEVICE device, UINT num_viewports, UINT clear_viewports,
        const D3D10_DDI_VIEWPORT* viewports)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(num_viewports);
    UNREFERENCED_PARAMETER(clear_viewports);
    UNREFERENCED_PARAMETER(viewports);
}

void APIENTRY umd_device::set_scissor_rects(D3D10DDI_HDEVICE device, UINT num_scissor_rects, UINT clear_scissor_rects,
        const D3D10_DDI_RECT* rects)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(num_scissor_rects);
    UNREFERENCED_PARAMETER(clear_scissor_rects);
    UNREFERENCED_PARAMETER(rects);
}

void APIENTRY umd_device::clear_render_target_view(D3D10DDI_HDEVICE device, D3D10DDI_HRENDERTARGETVIEW render_target_view,
        FLOAT color_rgba[4])
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(render_target_view);
    UNREFERENCED_PARAMETER(color_rgba);
}
void APIENTRY umd_device::clear_depth_stencil_view(D3D10DDI_HDEVICE device, D3D10DDI_HDEPTHSTENCILVIEW depth_stencil_view,
        UINT flags, FLOAT depth, UINT8 stencil)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(depth_stencil_view);
    UNREFERENCED_PARAMETER(flags);
    UNREFERENCED_PARAMETER(depth);
    UNREFERENCED_PARAMETER(stencil);
}

void APIENTRY umd_device::set_predication(D3D10DDI_HDEVICE device, D3D10DDI_HQUERY query, BOOL predicate_value)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(query);
    UNREFERENCED_PARAMETER(predicate_value);
}

void APIENTRY umd_device::query_get_data(D3D10DDI_HDEVICE device, D3D10DDI_HQUERY query, VOID* data, UINT data_size, UINT flags)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(query);
    UNREFERENCED_PARAMETER(data);
    UNREFERENCED_PARAMETER(data_size);
    UNREFERENCED_PARAMETER(flags);
}

void APIENTRY umd_device::flush(D3D10DDI_HDEVICE device)
{
    UNREFERENCED_PARAMETER(device);
}

void APIENTRY umd_device::gen_mips(D3D10DDI_HDEVICE device, D3D10DDI_HSHADERRESOURCEVIEW shader_resource_view)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(shader_resource_view);
}

void APIENTRY umd_device::resource_copy(D3D10DDI_HDEVICE device, D3D10DDI_HRESOURCE dst_resource, D3D10DDI_HRESOURCE src_resource)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(dst_resource);
    UNREFERENCED_PARAMETER(src_resource);
}

void APIENTRY umd_device::resource_resolve_subresource(D3D10DDI_HDEVICE device, D3D10DDI_HRESOURCE dst_resource,
        UINT dst_subresource, D3D10DDI_HRESOURCE src_resource, UINT src_subresource, DXGI_FORMAT resolve_format)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(dst_resource);
    UNREFERENCED_PARAMETER(dst_subresource);
    UNREFERENCED_PARAMETER(src_resource);
    UNREFERENCED_PARAMETER(src_subresource);
    UNREFERENCED_PARAMETER(resolve_format);
}

void umd_device::resource_map(D3D10DDI_HDEVICE device, D3D10DDI_HRESOURCE resource,
        UINT subresource, D3D10_DDI_MAP ddi_map, UINT flags,
        D3D10DDI_MAPPED_SUBRESOURCE* mapped_subresource)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(resource);
    UNREFERENCED_PARAMETER(subresource);
    UNREFERENCED_PARAMETER(ddi_map);
    UNREFERENCED_PARAMETER(flags);
    UNREFERENCED_PARAMETER(mapped_subresource);
}

void umd_device::resource_unmap(D3D10DDI_HDEVICE device, D3D10DDI_HRESOURCE resource,
        UINT subresource)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(resource);
    UNREFERENCED_PARAMETER(subresource);
}

BOOL umd_device::resource_is_staging_busy(D3D10DDI_HDEVICE device, D3D10DDI_HRESOURCE resource)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(resource);

    return FALSE;
}

void umd_device::relocate_device_funcs(D3D10DDI_HDEVICE device, D3D11DDI_DEVICEFUNCS* device_funcs)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(device_funcs);
}

SIZE_T umd_device::calc_private_resource_size(D3D10DDI_HDEVICE device, const D3D11DDIARG_CREATERESOURCE* create_resource)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(create_resource);

    return 0;
}

SIZE_T umd_device::calc_private_opened_resource_size(D3D10DDI_HDEVICE device, const D3D10DDIARG_OPENRESOURCE* open_resource)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(open_resource);

    return 0;
}

void umd_device::create_resource(D3D10DDI_HDEVICE device, const D3D11DDIARG_CREATERESOURCE* create_resource,
        D3D10DDI_HRESOURCE resource, D3D10DDI_HRTRESOURCE rt_resource)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(create_resource);
    UNREFERENCED_PARAMETER(resource);
    UNREFERENCED_PARAMETER(rt_resource);
}

void umd_device::open_resource(D3D10DDI_HDEVICE device, const D3D10DDIARG_OPENRESOURCE* open_resource,
        D3D10DDI_HRESOURCE resource, D3D10DDI_HRTRESOURCE rt_resource)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(open_resource);
    UNREFERENCED_PARAMETER(resource);
    UNREFERENCED_PARAMETER(rt_resource);
}

void umd_device::destroy_resource(D3D10DDI_HDEVICE device, D3D10DDI_HRESOURCE resource)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(resource);
}

SIZE_T umd_device::calc_private_shader_resource_view_size(D3D10DDI_HDEVICE device,
        const D3D11DDIARG_CREATESHADERRESOURCEVIEW* create_shader_resource_view)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(create_shader_resource_view);

    return 0;
}

void umd_device::create_shader_resource_view(D3D10DDI_HDEVICE device,
        const D3D11DDIARG_CREATESHADERRESOURCEVIEW* create_shader_resource_view,
        D3D10DDI_HSHADERRESOURCEVIEW shader_resource_view, D3D10DDI_HRTSHADERRESOURCEVIEW rt_shader_resource_view)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(create_shader_resource_view);
    UNREFERENCED_PARAMETER(shader_resource_view);
    UNREFERENCED_PARAMETER(rt_shader_resource_view);
}

void umd_device::destroy_shader_resource_view(D3D10DDI_HDEVICE device,
        D3D10DDI_HSHADERRESOURCEVIEW shader_resource_view)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(shader_resource_view);
}

SIZE_T umd_device::calc_private_render_target_view_size(D3D10DDI_HDEVICE device,
        const D3D10DDIARG_CREATERENDERTARGETVIEW* create_render_target_view)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(create_render_target_view);

    return 0;
}

void umd_device::create_render_target_view(D3D10DDI_HDEVICE device,
        const D3D10DDIARG_CREATERENDERTARGETVIEW* create_render_target_view, D3D10DDI_HRENDERTARGETVIEW render_target_view,
        D3D10DDI_HRTRENDERTARGETVIEW rt_render_target_view)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(create_render_target_view);
    UNREFERENCED_PARAMETER(render_target_view);
    UNREFERENCED_PARAMETER(rt_render_target_view);
}

void umd_device::destroy_render_target_view(D3D10DDI_HDEVICE device, D3D10DDI_HRENDERTARGETVIEW render_target_view)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(render_target_view);
}

SIZE_T umd_device::calc_private_depth_stencil_view_size(D3D10DDI_HDEVICE device,
        const D3D11DDIARG_CREATEDEPTHSTENCILVIEW* create_depth_stencil_view)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(create_depth_stencil_view);

    return 0;
}

void umd_device::create_depth_stencil_view(D3D10DDI_HDEVICE device,
        const D3D11DDIARG_CREATEDEPTHSTENCILVIEW* create_depth_stencil_view, D3D10DDI_HDEPTHSTENCILVIEW depth_stencil_view,
        D3D10DDI_HRTDEPTHSTENCILVIEW rt_depth_stencil_view)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(create_depth_stencil_view);
    UNREFERENCED_PARAMETER(depth_stencil_view);
    UNREFERENCED_PARAMETER(rt_depth_stencil_view);
}

void umd_device::destroy_depth_stencil_view(D3D10DDI_HDEVICE device, D3D10DDI_HDEPTHSTENCILVIEW depth_stencil_view)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(depth_stencil_view);
}

SIZE_T umd_device::calc_private_element_layout_size(D3D10DDI_HDEVICE device,
        const D3D10DDIARG_CREATEELEMENTLAYOUT* create_element_layout)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(create_element_layout);

    return 0;
}

void umd_device::create_element_layout(D3D10DDI_HDEVICE device,
        const D3D10DDIARG_CREATEELEMENTLAYOUT* create_element_layout, D3D10DDI_HELEMENTLAYOUT element_layout,
        D3D10DDI_HRTELEMENTLAYOUT rt_element_layout)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(create_element_layout);
    UNREFERENCED_PARAMETER(element_layout);
    UNREFERENCED_PARAMETER(rt_element_layout);
}

void umd_device::destroy_element_layout(D3D10DDI_HDEVICE device, D3D10DDI_HELEMENTLAYOUT element_layout)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(element_layout);
}

SIZE_T umd_device::calc_private_blend_state_size(D3D10DDI_HDEVICE device,
        const D3D10_1_DDI_BLEND_DESC* blend_desc)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(blend_desc);

    return 0;
}

void umd_device::create_blend_state(D3D10DDI_HDEVICE device,
        const D3D10_1_DDI_BLEND_DESC* blend_desc, D3D10DDI_HBLENDSTATE blend_state,
        D3D10DDI_HRTBLENDSTATE rt_blend_state)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(blend_desc);
    UNREFERENCED_PARAMETER(blend_state);
    UNREFERENCED_PARAMETER(rt_blend_state);
}

void umd_device::destroy_blend_state(D3D10DDI_HDEVICE device, D3D10DDI_HBLENDSTATE blend_state)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(blend_state);
}

SIZE_T umd_device::calc_private_depth_stencil_state_size(D3D10DDI_HDEVICE device,
        const D3D10_DDI_DEPTH_STENCIL_DESC* depth_stencil_desc)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(depth_stencil_desc);

    return 0;
}

void umd_device::create_depth_stencil_state(D3D10DDI_HDEVICE device,
        const D3D10_DDI_DEPTH_STENCIL_DESC* depth_stencil_desc, D3D10DDI_HDEPTHSTENCILSTATE depth_stencil_state,
        D3D10DDI_HRTDEPTHSTENCILSTATE rt_depth_stencil_state)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(depth_stencil_desc);
    UNREFERENCED_PARAMETER(depth_stencil_state);
    UNREFERENCED_PARAMETER(rt_depth_stencil_state);
}

void umd_device::destroy_depth_stencil_state(D3D10DDI_HDEVICE device, D3D10DDI_HDEPTHSTENCILSTATE depth_stencil_state)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(depth_stencil_state);
}

SIZE_T umd_device::calc_private_rasterizer_stateSize(D3D10DDI_HDEVICE device,
        const D3D10_DDI_RASTERIZER_DESC* rasterizer_desc)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(rasterizer_desc);

    return 0;
}

void umd_device::create_rasterizer_state(D3D10DDI_HDEVICE device,
        const D3D10_DDI_RASTERIZER_DESC* rasterizer_desc, D3D10DDI_HRASTERIZERSTATE rasterizer_state,
        D3D10DDI_HRTRASTERIZERSTATE rt_rasterizer_state)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(rasterizer_desc);
    UNREFERENCED_PARAMETER(rasterizer_state);
    UNREFERENCED_PARAMETER(rt_rasterizer_state);
}

void umd_device::destroy_rasterizer_state(D3D10DDI_HDEVICE device, D3D10DDI_HRASTERIZERSTATE rasterizer_state)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(rasterizer_state);
}

SIZE_T umd_device::calc_private_shader_size(D3D10DDI_HDEVICE device, const UINT* code,
        const D3D10DDIARG_STAGE_IO_SIGNATURES* signatures)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(code);
    UNREFERENCED_PARAMETER(signatures);

    return 0;
}

void umd_device::create_vertex_shader(D3D10DDI_HDEVICE device, const UINT* code,
        D3D10DDI_HSHADER shader, D3D10DDI_HRTSHADER rt_shader, const D3D10DDIARG_STAGE_IO_SIGNATURES* signatures)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(code);
    UNREFERENCED_PARAMETER(shader);
    UNREFERENCED_PARAMETER(rt_shader);
    UNREFERENCED_PARAMETER(signatures);
}

void umd_device::create_geometry_shader(D3D10DDI_HDEVICE device, const UINT* code,
        D3D10DDI_HSHADER shader, D3D10DDI_HRTSHADER rt_shader, const D3D10DDIARG_STAGE_IO_SIGNATURES* signatures)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(code);
    UNREFERENCED_PARAMETER(shader);
    UNREFERENCED_PARAMETER(rt_shader);
    UNREFERENCED_PARAMETER(signatures);
}

void umd_device::create_pixel_shader(D3D10DDI_HDEVICE device, const UINT* code,
        D3D10DDI_HSHADER shader, D3D10DDI_HRTSHADER rt_shader, const D3D10DDIARG_STAGE_IO_SIGNATURES* signatures)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(code);
    UNREFERENCED_PARAMETER(shader);
    UNREFERENCED_PARAMETER(rt_shader);
    UNREFERENCED_PARAMETER(signatures);
}

SIZE_T umd_device::calc_private_geometry_shader_with_stream_output(D3D10DDI_HDEVICE device,
        const D3D11DDIARG_CREATEGEOMETRYSHADERWITHSTREAMOUTPUT* create_geometry_shader_with_stream_output,
        const D3D10DDIARG_STAGE_IO_SIGNATURES* signatures)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(create_geometry_shader_with_stream_output);
    UNREFERENCED_PARAMETER(signatures);

    return 0;
}

void umd_device::create_geometry_shader_with_stream_output(D3D10DDI_HDEVICE device,
        const D3D11DDIARG_CREATEGEOMETRYSHADERWITHSTREAMOUTPUT* create_geometry_shader_with_stream_output,
        D3D10DDI_HSHADER shader, D3D10DDI_HRTSHADER rt_shader, const D3D10DDIARG_STAGE_IO_SIGNATURES* signatures)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(create_geometry_shader_with_stream_output);
    UNREFERENCED_PARAMETER(shader);
    UNREFERENCED_PARAMETER(rt_shader);
    UNREFERENCED_PARAMETER(signatures);
}

void umd_device::destroy_shader(D3D10DDI_HDEVICE device, D3D10DDI_HSHADER shader)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(shader);
}

SIZE_T umd_device::calc_private_sampler_size(D3D10DDI_HDEVICE device,
        const D3D10_DDI_SAMPLER_DESC* sampler_desc)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(sampler_desc);

    return 0;
}

void umd_device::create_sampler(D3D10DDI_HDEVICE device,
        const D3D10_DDI_SAMPLER_DESC* sampler_desc, D3D10DDI_HSAMPLER sampler,
        D3D10DDI_HRTSAMPLER rt_sampler)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(sampler_desc);
    UNREFERENCED_PARAMETER(sampler);
    UNREFERENCED_PARAMETER(rt_sampler);
}

void umd_device::destroy_sampler(D3D10DDI_HDEVICE device, D3D10DDI_HSAMPLER sampler)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(sampler);
}

SIZE_T umd_device::calc_private_query_size(D3D10DDI_HDEVICE device,
        const D3D10DDIARG_CREATEQUERY* create_query)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(create_query);

    return 0;
}

void umd_device::create_query(D3D10DDI_HDEVICE device,
        const D3D10DDIARG_CREATEQUERY *create_query, D3D10DDI_HQUERY query,
        D3D10DDI_HRTQUERY rt_query)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(create_query);
    UNREFERENCED_PARAMETER(query);
    UNREFERENCED_PARAMETER(rt_query);
}

void umd_device::destroy_query(D3D10DDI_HDEVICE device, D3D10DDI_HQUERY query)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(query);
}

void umd_device::check_format_support(D3D10DDI_HDEVICE device,
        DXGI_FORMAT format, UINT* format_caps)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(format);
    UNREFERENCED_PARAMETER(format_caps);
}

void umd_device::check_multisample_quality_levels(D3D10DDI_HDEVICE device,
        DXGI_FORMAT format, UINT sample_count, UINT* num_quality_levels)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(format);
    UNREFERENCED_PARAMETER(sample_count);
    UNREFERENCED_PARAMETER(num_quality_levels);
}

void umd_device::check_counter_info(D3D10DDI_HDEVICE device,
        D3D10DDI_COUNTER_INFO* counter_info)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(counter_info);
}

void umd_device::check_counter(D3D10DDI_HDEVICE device,
        D3D10DDI_QUERY query,  D3D10DDI_COUNTER_TYPE* counter_type,
        UINT* active_counters, LPSTR name, UINT* name_length, LPSTR units, UINT* units_length,
        LPSTR description, UINT* description_length)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(query);
    UNREFERENCED_PARAMETER(counter_type);
    UNREFERENCED_PARAMETER(active_counters);
    UNREFERENCED_PARAMETER(name);
    UNREFERENCED_PARAMETER(name_length);
    UNREFERENCED_PARAMETER(units);
    UNREFERENCED_PARAMETER(units_length);
    UNREFERENCED_PARAMETER(description);
    UNREFERENCED_PARAMETER(description_length);
}

void umd_device::destroy_device(D3D10DDI_HDEVICE device)
{
    UNREFERENCED_PARAMETER(device);
}

void umd_device::set_text_filter_size(D3D10DDI_HDEVICE device,
        UINT width, UINT height)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(width);
    UNREFERENCED_PARAMETER(height);
}

void umd_device::resource_convert(D3D10DDI_HDEVICE device, D3D10DDI_HRESOURCE dst_resource,
        D3D10DDI_HRESOURCE src_resource)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(dst_resource);
    UNREFERENCED_PARAMETER(src_resource);
}

void umd_device::resource_convert_region(D3D10DDI_HDEVICE device, D3D10DDI_HRESOURCE dst_resource,
        UINT dst_subresource, UINT dst_x, UINT dst_y, UINT dst_z,
        D3D10DDI_HRESOURCE src_resource, UINT src_subresource, const D3D10_DDI_BOX* src_box)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(dst_resource);
    UNREFERENCED_PARAMETER(dst_subresource);
    UNREFERENCED_PARAMETER(dst_x);
    UNREFERENCED_PARAMETER(dst_y);
    UNREFERENCED_PARAMETER(dst_z);
    UNREFERENCED_PARAMETER(src_resource);
    UNREFERENCED_PARAMETER(src_subresource);
    UNREFERENCED_PARAMETER(src_box);
}

void umd_device::draw_indexed_instanced_indirect(D3D10DDI_HDEVICE device,
        D3D10DDI_HRESOURCE buffer_for_args, UINT aligned_byte_offset_for_args)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(buffer_for_args);
    UNREFERENCED_PARAMETER(aligned_byte_offset_for_args);
}

void umd_device::draw_instanced_indirect(D3D10DDI_HDEVICE device,
        D3D10DDI_HRESOURCE buffer_for_args, UINT aligned_byte_offset_for_args)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(buffer_for_args);
    UNREFERENCED_PARAMETER(aligned_byte_offset_for_args);
}

void umd_device::command_list_execute(D3D10DDI_HDEVICE device,
        D3D11DDI_HCOMMANDLIST command_list)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(command_list);
}

void umd_device::hs_set_shader_resources(D3D10DDI_HDEVICE device, UINT offset, UINT num_views,
        const D3D10DDI_HSHADERRESOURCEVIEW* shader_resource_views)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(offset);
    UNREFERENCED_PARAMETER(num_views);
    UNREFERENCED_PARAMETER(shader_resource_views);
}

void umd_device::hs_set_shader(D3D10DDI_HDEVICE device, D3D10DDI_HSHADER shader)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(shader);
}

void umd_device::hs_set_samplers(D3D10DDI_HDEVICE device, UINT offset, UINT num_samplers,
        const D3D10DDI_HSAMPLER* samplers)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(offset);
    UNREFERENCED_PARAMETER(num_samplers);
    UNREFERENCED_PARAMETER(samplers);
}

void umd_device::hs_set_constant_buffers(D3D10DDI_HDEVICE device, UINT start_buffer, UINT num_buffers,
        const D3D10DDI_HRESOURCE* buffers)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(start_buffer);
    UNREFERENCED_PARAMETER(num_buffers);
    UNREFERENCED_PARAMETER(buffers);
}

void umd_device::ds_set_shader_resources(D3D10DDI_HDEVICE device, UINT offset, UINT num_views,
        const D3D10DDI_HSHADERRESOURCEVIEW *shader_resource_views)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(offset);
    UNREFERENCED_PARAMETER(num_views);
    UNREFERENCED_PARAMETER(shader_resource_views);
}

void umd_device::ds_set_shader(D3D10DDI_HDEVICE device, D3D10DDI_HSHADER shader)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(shader);
}

void umd_device::ds_set_samplers(D3D10DDI_HDEVICE device, UINT offset, UINT num_samplers,
        const D3D10DDI_HSAMPLER* samplers)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(offset);
    UNREFERENCED_PARAMETER(num_samplers);
    UNREFERENCED_PARAMETER(samplers);
}

void umd_device::ds_set_constant_buffers(D3D10DDI_HDEVICE device, UINT start_buffer, UINT num_buffers,
        const D3D10DDI_HRESOURCE* buffers)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(start_buffer);
    UNREFERENCED_PARAMETER(num_buffers);
    UNREFERENCED_PARAMETER(buffers);
}

void umd_device::create_hull_shader(D3D10DDI_HDEVICE device, const UINT* code,
        D3D10DDI_HSHADER shader, D3D10DDI_HRTSHADER rt_shader, const D3D11DDIARG_TESSELLATION_IO_SIGNATURES* signatures)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(code);
    UNREFERENCED_PARAMETER(shader);
    UNREFERENCED_PARAMETER(rt_shader);
    UNREFERENCED_PARAMETER(signatures);
}

void umd_device::create_domain_shader(D3D10DDI_HDEVICE device, const UINT* code,
        D3D10DDI_HSHADER shader, D3D10DDI_HRTSHADER rt_shader, const D3D11DDIARG_TESSELLATION_IO_SIGNATURES* signatures)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(code);
    UNREFERENCED_PARAMETER(shader);
    UNREFERENCED_PARAMETER(rt_shader);
    UNREFERENCED_PARAMETER(signatures);
}

void umd_device::check_deferred_context_handle_sizes(D3D10DDI_HDEVICE device, UINT* h_sizes,
        D3D11DDI_HANDLESIZE* handle_size)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(h_sizes);
    UNREFERENCED_PARAMETER(handle_size);
}

SIZE_T umd_device::calc_deferred_context_handle_size(D3D10DDI_HDEVICE device, D3D11DDI_HANDLETYPE handle_type,
        void* ic_object)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(handle_type);
    UNREFERENCED_PARAMETER(ic_object);

    return 0;
}

SIZE_T umd_device::calc_private_deferred_context_size(D3D10DDI_HDEVICE device,
        const D3D11DDIARG_CALCPRIVATEDEFERREDCONTEXTSIZE* calc_private_deferred_context_size)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(calc_private_deferred_context_size);

    return 0;
}

void umd_device::create_deferred_context(D3D10DDI_HDEVICE device,
        const D3D11DDIARG_CREATEDEFERREDCONTEXT* create_deferred_context)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(create_deferred_context);
}

void umd_device::abandon_command_list(D3D10DDI_HDEVICE device)
{
    UNREFERENCED_PARAMETER(device);
}

SIZE_T umd_device::calc_private_command_list_size(D3D10DDI_HDEVICE device,
        const D3D11DDIARG_CREATECOMMANDLIST* create_command_list)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(create_command_list);

    return 0;
}

void umd_device::create_command_list(D3D10DDI_HDEVICE device,
        const D3D11DDIARG_CREATECOMMANDLIST* create_command_list, D3D11DDI_HCOMMANDLIST command_list,
        D3D11DDI_HRTCOMMANDLIST rt_command_list)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(create_command_list);
    UNREFERENCED_PARAMETER(command_list);
    UNREFERENCED_PARAMETER(rt_command_list);
}

void umd_device::destroy_command_list(D3D10DDI_HDEVICE device, D3D11DDI_HCOMMANDLIST command_list)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(command_list);
}

SIZE_T umd_device::calc_private_tessellation_shader_size(D3D10DDI_HDEVICE device, const UINT* code,
        const D3D11DDIARG_TESSELLATION_IO_SIGNATURES* signatures)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(code);
    UNREFERENCED_PARAMETER(signatures);

    return 0;
}

void umd_device::ps_set_shader_with_ifaces(D3D10DDI_HDEVICE device, D3D10DDI_HSHADER shader,
        UINT num_class_instances, const UINT* ifaces, const D3D11DDIARG_POINTERDATA* pointer_data)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(shader);
    UNREFERENCED_PARAMETER(num_class_instances);
    UNREFERENCED_PARAMETER(ifaces);
    UNREFERENCED_PARAMETER(pointer_data);
}

void umd_device::vs_set_shader_with_ifaces(D3D10DDI_HDEVICE device, D3D10DDI_HSHADER shader,
        UINT num_class_instances, const UINT* ifaces, const D3D11DDIARG_POINTERDATA* pointer_data)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(shader);
    UNREFERENCED_PARAMETER(num_class_instances);
    UNREFERENCED_PARAMETER(ifaces);
    UNREFERENCED_PARAMETER(pointer_data);
}

void umd_device::gs_set_shader_with_ifaces(D3D10DDI_HDEVICE device, D3D10DDI_HSHADER shader,
        UINT num_class_instances, const UINT* ifaces, const D3D11DDIARG_POINTERDATA* pointer_data)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(shader);
    UNREFERENCED_PARAMETER(num_class_instances);
    UNREFERENCED_PARAMETER(ifaces);
    UNREFERENCED_PARAMETER(pointer_data);
}

void umd_device::hs_set_shader_with_ifaces(D3D10DDI_HDEVICE device, D3D10DDI_HSHADER shader,
        UINT num_class_instances, const UINT* ifaces, const D3D11DDIARG_POINTERDATA* pointer_data)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(shader);
    UNREFERENCED_PARAMETER(num_class_instances);
    UNREFERENCED_PARAMETER(ifaces);
    UNREFERENCED_PARAMETER(pointer_data);
}

void umd_device::ds_set_shader_with_ifaces(D3D10DDI_HDEVICE device, D3D10DDI_HSHADER shader,
        UINT num_class_instances, const UINT* ifaces, const D3D11DDIARG_POINTERDATA* pointer_data)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(shader);
    UNREFERENCED_PARAMETER(num_class_instances);
    UNREFERENCED_PARAMETER(ifaces);
    UNREFERENCED_PARAMETER(pointer_data);
}

void umd_device::cs_set_shader_with_ifaces(D3D10DDI_HDEVICE device, D3D10DDI_HSHADER shader,
        UINT num_class_instances, const UINT* ifaces, const D3D11DDIARG_POINTERDATA* pointer_data)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(shader);
    UNREFERENCED_PARAMETER(num_class_instances);
    UNREFERENCED_PARAMETER(ifaces);
    UNREFERENCED_PARAMETER(pointer_data);
}

void umd_device::create_compute_shader(D3D10DDI_HDEVICE device, const UINT* code,
        D3D10DDI_HSHADER shader, D3D10DDI_HRTSHADER rt_shader)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(code);
    UNREFERENCED_PARAMETER(shader);
    UNREFERENCED_PARAMETER(rt_shader);
}

void umd_device::cs_set_shader(D3D10DDI_HDEVICE device, D3D10DDI_HSHADER shader)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(shader);
}

void umd_device::cs_set_shader_resources(D3D10DDI_HDEVICE device, UINT offset, UINT num_views,
        const D3D10DDI_HSHADERRESOURCEVIEW* shader_resource_views)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(offset);
    UNREFERENCED_PARAMETER(num_views);
    UNREFERENCED_PARAMETER(shader_resource_views);
}

void umd_device::cs_set_samplers(D3D10DDI_HDEVICE device, UINT offset, UINT num_samplers,
        const D3D10DDI_HSAMPLER* samplers)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(offset);
    UNREFERENCED_PARAMETER(num_samplers);
    UNREFERENCED_PARAMETER(samplers);
}

void umd_device::cs_set_constant_buffers(D3D10DDI_HDEVICE device, UINT start_buffer, UINT num_buffers,
        const D3D10DDI_HRESOURCE* buffers)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(start_buffer);
    UNREFERENCED_PARAMETER(num_buffers);
    UNREFERENCED_PARAMETER(buffers);
}

SIZE_T umd_device::calc_private_unordered_access_view_size(D3D10DDI_HDEVICE device,
        const D3D11DDIARG_CREATEUNORDEREDACCESSVIEW* create_unordered_access_view)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(create_unordered_access_view);

    return 0;
}

void umd_device::create_unordered_access_view(D3D10DDI_HDEVICE device,
        const D3D11DDIARG_CREATEUNORDEREDACCESSVIEW* create_unordered_access_view,
        D3D11DDI_HUNORDEREDACCESSVIEW unordered_access_view, D3D11DDI_HRTUNORDEREDACCESSVIEW rt_unordered_access_view)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(create_unordered_access_view);
    UNREFERENCED_PARAMETER(unordered_access_view);
    UNREFERENCED_PARAMETER(rt_unordered_access_view);
}

void umd_device::destroy_unordered_access_view(D3D10DDI_HDEVICE device,
        D3D11DDI_HUNORDEREDACCESSVIEW unordered_access_view)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(unordered_access_view);
}

void umd_device::clear_unordered_access_view_uint(D3D10DDI_HDEVICE device,
        D3D11DDI_HUNORDEREDACCESSVIEW unordered_access_view, const UINT uints[4])
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(unordered_access_view);
    UNREFERENCED_PARAMETER(uints);
}

void umd_device::clear_unordered_access_view_float(D3D10DDI_HDEVICE device,
        D3D11DDI_HUNORDEREDACCESSVIEW unordered_access_view, const FLOAT floats[4])
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(unordered_access_view);
    UNREFERENCED_PARAMETER(floats);
}

void umd_device::cs_set_unordered_access_views(D3D10DDI_HDEVICE device, UINT offset, UINT num_views,
        const D3D11DDI_HUNORDEREDACCESSVIEW* unordered_access_view, const UINT* uav_initial_counts)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(offset);
    UNREFERENCED_PARAMETER(num_views);
    UNREFERENCED_PARAMETER(unordered_access_view);
    UNREFERENCED_PARAMETER(uav_initial_counts);
}

void umd_device::dispatch(D3D10DDI_HDEVICE device, UINT thread_group_count_x,
        UINT thread_group_count_y, UINT thread_group_count_z)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(thread_group_count_x);
    UNREFERENCED_PARAMETER(thread_group_count_y);
    UNREFERENCED_PARAMETER(thread_group_count_z);
}

void umd_device::dispatch_indirect(D3D10DDI_HDEVICE device,
        D3D10DDI_HRESOURCE buffer_for_args, UINT aligned_byte_offset_for_args)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(buffer_for_args);
    UNREFERENCED_PARAMETER(aligned_byte_offset_for_args);
}

void umd_device::set_resource_min_lod(D3D10DDI_HDEVICE device, D3D10DDI_HRESOURCE resource,
        FLOAT min_lod)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(resource);
    UNREFERENCED_PARAMETER(min_lod);
}

void umd_device::copy_structure_count(D3D10DDI_HDEVICE device, D3D10DDI_HRESOURCE dst_buffer,
        UINT dst_aligned_byte_offset, D3D11DDI_HUNORDEREDACCESSVIEW src_view)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(dst_buffer);
    UNREFERENCED_PARAMETER(dst_aligned_byte_offset);
    UNREFERENCED_PARAMETER(src_view);
}

void umd_device::recycle_command_list(D3D10DDI_HDEVICE device, D3D11DDI_HCOMMANDLIST command_list)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(command_list);
}

HRESULT umd_device::recycle_create_command_list(D3D10DDI_HDEVICE device,
        const D3D11DDIARG_CREATECOMMANDLIST* create_command_list, D3D11DDI_HCOMMANDLIST command_list,
        D3D11DDI_HRTCOMMANDLIST rt_command_list)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(create_command_list);
    UNREFERENCED_PARAMETER(command_list);
    UNREFERENCED_PARAMETER(rt_command_list);

    return E_NOTIMPL;
}

HRESULT umd_device::recycle_create_deferred_context(D3D10DDI_HDEVICE device,
        const D3D11DDIARG_CREATEDEFERREDCONTEXT* create_deferred_context)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(create_deferred_context);

    return E_NOTIMPL;
}

void umd_device::recycle_destroy_command_list(D3D10DDI_HDEVICE device, D3D11DDI_HCOMMANDLIST command_list)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(command_list);
}

HRESULT umd_device::present_dxgi(DXGI_DDI_ARG_PRESENT* present_data)
{
    UNREFERENCED_PARAMETER(present_data);

    return E_NOTIMPL;
}

HRESULT umd_device::get_gamma_caps_dxgi(DXGI_DDI_ARG_GET_GAMMA_CONTROL_CAPS* gamma_data)
{
    UNREFERENCED_PARAMETER(gamma_data);

    return E_NOTIMPL;
}

HRESULT umd_device::set_display_mode_dxgi(DXGI_DDI_ARG_SETDISPLAYMODE* display_mode_data)
{
    UNREFERENCED_PARAMETER(display_mode_data);

    return E_NOTIMPL;
}

HRESULT umd_device::set_resource_priority_dxgi(DXGI_DDI_ARG_SETRESOURCEPRIORITY* priority_data)
{
    UNREFERENCED_PARAMETER(priority_data);

    return E_NOTIMPL;
}

HRESULT umd_device::query_resource_residency_dxgi(DXGI_DDI_ARG_QUERYRESOURCERESIDENCY* residency_data)
{
    UNREFERENCED_PARAMETER(residency_data);

    return E_NOTIMPL;
}

HRESULT umd_device::rotate_resource_identities_dxgi(DXGI_DDI_ARG_ROTATE_RESOURCE_IDENTITIES* rotate_data)
{
    UNREFERENCED_PARAMETER(rotate_data);

    return E_NOTIMPL;
}

HRESULT umd_device::blt_dxgi(DXGI_DDI_ARG_BLT* blt_data)
{
    UNREFERENCED_PARAMETER(blt_data);

    return E_NOTIMPL;
}

HRESULT umd_device::resolve_shared_resource_dxgi(DXGI_DDI_ARG_RESOLVESHAREDRESOURCE* resource_data)
{
    UNREFERENCED_PARAMETER(resource_data);

    return E_NOTIMPL;
}

HRESULT umd_device::blt1_dxgi(DXGI_DDI_ARG_BLT1* blt1_data)
{
    UNREFERENCED_PARAMETER(blt1_data);

    return E_NOTIMPL;
}

HRESULT umd_device::offer_resources(DXGI_DDI_ARG_OFFERRESOURCES* resources)
{
    UNREFERENCED_PARAMETER(resources);

    return E_NOTIMPL;
}

HRESULT umd_device::reclaim_resources(DXGI_DDI_ARG_RECLAIMRESOURCES* resources)
{
    UNREFERENCED_PARAMETER(resources);

    return E_NOTIMPL;
}

HRESULT umd_device::get_multi_plane_overlay_caps(DXGI_DDI_ARG_GETMULTIPLANEOVERLAYCAPS* caps)
{
    UNREFERENCED_PARAMETER(caps);

    return E_NOTIMPL;
}

HRESULT umd_device::check_multi_plane_overlay_support(DXGI_DDI_ARG_CHECKMULTIPLANEOVERLAYSUPPORT* support)
{
    UNREFERENCED_PARAMETER(support);

    return E_NOTIMPL;
}

HRESULT umd_device::present_multi_plane_overlay(DXGI_DDI_ARG_PRESENTMULTIPLANEOVERLAY* present_dxgi)
{
    UNREFERENCED_PARAMETER(present_dxgi);

    return E_NOTIMPL;
}
