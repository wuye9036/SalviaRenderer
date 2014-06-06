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
        d3d11_device_funcs_->pfnCalcPrivateRenderTargetViewSize = nullptr;
        d3d11_device_funcs_->pfnCreateRenderTargetView = nullptr;
        d3d11_device_funcs_->pfnDestroyRenderTargetView = nullptr;
        d3d11_device_funcs_->pfnCalcPrivateDepthStencilViewSize = nullptr;
        d3d11_device_funcs_->pfnCreateDepthStencilView = nullptr;
        d3d11_device_funcs_->pfnDestroyDepthStencilView = nullptr;
        d3d11_device_funcs_->pfnCalcPrivateElementLayoutSize = nullptr;
        d3d11_device_funcs_->pfnCreateElementLayout = nullptr;
        d3d11_device_funcs_->pfnDestroyElementLayout = nullptr;
        d3d11_device_funcs_->pfnCalcPrivateBlendStateSize = nullptr;
        d3d11_device_funcs_->pfnCreateBlendState = nullptr;
        d3d11_device_funcs_->pfnDestroyBlendState = nullptr;
        d3d11_device_funcs_->pfnCalcPrivateDepthStencilStateSize = nullptr;
        d3d11_device_funcs_->pfnCreateDepthStencilState = nullptr;
        d3d11_device_funcs_->pfnDestroyDepthStencilState = nullptr;
        d3d11_device_funcs_->pfnCalcPrivateRasterizerStateSize = nullptr;
        d3d11_device_funcs_->pfnCreateRasterizerState = nullptr;
        d3d11_device_funcs_->pfnDestroyRasterizerState = nullptr;
        d3d11_device_funcs_->pfnCalcPrivateShaderSize = nullptr;
        d3d11_device_funcs_->pfnCreateVertexShader = nullptr;
        d3d11_device_funcs_->pfnCreateGeometryShader = nullptr;
        d3d11_device_funcs_->pfnCreatePixelShader = nullptr;
        d3d11_device_funcs_->pfnCalcPrivateGeometryShaderWithStreamOutput = nullptr;
        d3d11_device_funcs_->pfnCreateGeometryShaderWithStreamOutput = nullptr;
        d3d11_device_funcs_->pfnDestroyShader = nullptr;
        d3d11_device_funcs_->pfnCalcPrivateSamplerSize = nullptr;
        d3d11_device_funcs_->pfnCreateSampler = nullptr;
        d3d11_device_funcs_->pfnDestroySampler = nullptr;
        d3d11_device_funcs_->pfnCalcPrivateQuerySize = nullptr;
        d3d11_device_funcs_->pfnCreateQuery = nullptr;
        d3d11_device_funcs_->pfnDestroyQuery = nullptr;
        d3d11_device_funcs_->pfnCheckFormatSupport = nullptr;
        d3d11_device_funcs_->pfnCheckMultisampleQualityLevels = nullptr;
        d3d11_device_funcs_->pfnCheckCounterInfo = nullptr;
        d3d11_device_funcs_->pfnCheckCounter = nullptr;
        d3d11_device_funcs_->pfnDestroyDevice = nullptr;
        d3d11_device_funcs_->pfnSetTextFilterSize = nullptr;

        // Additional 10.1 entries
        d3d11_device_funcs_->pfnResourceConvert = nullptr;
        d3d11_device_funcs_->pfnResourceConvertRegion = nullptr;

        // Additional 11.0 entries
        d3d11_device_funcs_->pfnDrawIndexedInstancedIndirect = nullptr;
        d3d11_device_funcs_->pfnDrawInstancedIndirect = nullptr;
        d3d11_device_funcs_->pfnCommandListExecute = nullptr;
        d3d11_device_funcs_->pfnHsSetShaderResources = nullptr;
        d3d11_device_funcs_->pfnHsSetShader = nullptr;
        d3d11_device_funcs_->pfnHsSetSamplers = nullptr;
        d3d11_device_funcs_->pfnHsSetConstantBuffers = nullptr;
        d3d11_device_funcs_->pfnDsSetShaderResources = nullptr;
        d3d11_device_funcs_->pfnDsSetShader = nullptr;
        d3d11_device_funcs_->pfnDsSetSamplers = nullptr;
        d3d11_device_funcs_->pfnDsSetConstantBuffers = nullptr;
        d3d11_device_funcs_->pfnCreateHullShader = nullptr;
        d3d11_device_funcs_->pfnCreateDomainShader = nullptr;
        d3d11_device_funcs_->pfnCheckDeferredContextHandleSizes = nullptr;
        d3d11_device_funcs_->pfnCalcDeferredContextHandleSize = nullptr;
        d3d11_device_funcs_->pfnCalcPrivateDeferredContextSize = nullptr;
        d3d11_device_funcs_->pfnCreateDeferredContext = nullptr;
        d3d11_device_funcs_->pfnAbandonCommandList = nullptr;
        d3d11_device_funcs_->pfnCalcPrivateCommandListSize = nullptr;
        d3d11_device_funcs_->pfnCreateCommandList = nullptr;
        d3d11_device_funcs_->pfnDestroyCommandList = nullptr;
        d3d11_device_funcs_->pfnCalcPrivateTessellationShaderSize = nullptr;
        d3d11_device_funcs_->pfnPsSetShaderWithIfaces = nullptr;
        d3d11_device_funcs_->pfnVsSetShaderWithIfaces = nullptr;
        d3d11_device_funcs_->pfnGsSetShaderWithIfaces = nullptr;
        d3d11_device_funcs_->pfnHsSetShaderWithIfaces = nullptr;
        d3d11_device_funcs_->pfnDsSetShaderWithIfaces = nullptr;
        d3d11_device_funcs_->pfnCsSetShaderWithIfaces = nullptr;
        d3d11_device_funcs_->pfnCreateComputeShader = nullptr;
        d3d11_device_funcs_->pfnCsSetShader = nullptr;
        d3d11_device_funcs_->pfnCsSetShaderResources = nullptr;
        d3d11_device_funcs_->pfnCsSetSamplers = nullptr;
        d3d11_device_funcs_->pfnCsSetConstantBuffers = nullptr;
        d3d11_device_funcs_->pfnCalcPrivateUnorderedAccessViewSize = nullptr;
        d3d11_device_funcs_->pfnCreateUnorderedAccessView = nullptr;
        d3d11_device_funcs_->pfnDestroyUnorderedAccessView = nullptr;
        d3d11_device_funcs_->pfnClearUnorderedAccessViewUint = nullptr;
        d3d11_device_funcs_->pfnClearUnorderedAccessViewFloat = nullptr;
        d3d11_device_funcs_->pfnCsSetUnorderedAccessViews = nullptr;
        d3d11_device_funcs_->pfnDispatch = nullptr;
        d3d11_device_funcs_->pfnDispatchIndirect = nullptr;
        d3d11_device_funcs_->pfnSetResourceMinLOD = nullptr;
        d3d11_device_funcs_->pfnCopyStructureCount = nullptr;

        // D3D11_0_*DDI_BUILD_VERSION == 2
        d3d11_device_funcs_->pfnRecycleCommandList = nullptr;
        d3d11_device_funcs_->pfnRecycleCreateCommandList = nullptr;
        d3d11_device_funcs_->pfnRecycleCreateDeferredContext = nullptr;
        d3d11_device_funcs_->pfnRecycleDestroyCommandList = nullptr;
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
    dxgi_ddi_funcs->pfnPresent = nullptr;
    dxgi_ddi_funcs->pfnGetGammaCaps = nullptr;
    dxgi_ddi_funcs->pfnSetDisplayMode = nullptr;
    dxgi_ddi_funcs->pfnSetResourcePriority = nullptr;
    dxgi_ddi_funcs->pfnQueryResourceResidency = nullptr;
    dxgi_ddi_funcs->pfnRotateResourceIdentities = nullptr;
    dxgi_ddi_funcs->pfnBlt = nullptr;
    dxgi_ddi_funcs->pfnResolveSharedResource = nullptr;
#if (D3D_UMD_INTERFACE_VERSION >= D3D_UMD_INTERFACE_VERSION_WIN8)
    dxgi_ddi_funcs->pfnBlt1 = nullptr;
    dxgi_ddi_funcs->pfnOfferResources = nullptr;
    dxgi_ddi_funcs->pfnReclaimResources = nullptr;
    dxgi_ddi_funcs->pfnGetMultiPlaneOverlayCaps = nullptr;
    dxgi_ddi_funcs->pfnGetMultiPlaneOverlayFilterRange = nullptr;
    dxgi_ddi_funcs->pfnCheckMultiPlaneOverlaySupport = nullptr;
    dxgi_ddi_funcs->pfnPresentMultiPlaneOverlay = nullptr;
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

BOOL APIENTRY umd_device::resource_is_staging_busy(D3D10DDI_HDEVICE device, D3D10DDI_HRESOURCE resource)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(resource);

    return FALSE;
}

void APIENTRY umd_device::relocate_device_funcs(D3D10DDI_HDEVICE device, D3D11DDI_DEVICEFUNCS* device_funcs)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(device_funcs);
}

SIZE_T APIENTRY umd_device::calc_private_resource_size(D3D10DDI_HDEVICE device, const D3D11DDIARG_CREATERESOURCE* create_resource)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(create_resource);

    return 0;
}

SIZE_T APIENTRY umd_device::calc_private_opened_resource_size(D3D10DDI_HDEVICE device, const D3D10DDIARG_OPENRESOURCE* open_resource)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(open_resource);

    return 0;
}

void APIENTRY umd_device::create_resource(D3D10DDI_HDEVICE device, const D3D11DDIARG_CREATERESOURCE* create_resource,
        D3D10DDI_HRESOURCE resource, D3D10DDI_HRTRESOURCE rt_resource)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(create_resource);
    UNREFERENCED_PARAMETER(resource);
    UNREFERENCED_PARAMETER(rt_resource);
}

void APIENTRY umd_device::open_resource(D3D10DDI_HDEVICE device, const D3D10DDIARG_OPENRESOURCE* open_resource,
        D3D10DDI_HRESOURCE resource, D3D10DDI_HRTRESOURCE rt_resource)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(open_resource);
    UNREFERENCED_PARAMETER(resource);
    UNREFERENCED_PARAMETER(rt_resource);
}

void APIENTRY umd_device::destroy_resource(D3D10DDI_HDEVICE device, D3D10DDI_HRESOURCE resource)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(resource);
}

SIZE_T APIENTRY umd_device::calc_private_shader_resource_view_size(D3D10DDI_HDEVICE device,
        const D3D11DDIARG_CREATESHADERRESOURCEVIEW* create_shader_resource_view)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(create_shader_resource_view);

    return 0;
}

void APIENTRY umd_device::create_shader_resource_view(D3D10DDI_HDEVICE device,
        const D3D11DDIARG_CREATESHADERRESOURCEVIEW* create_shader_resource_view,
        D3D10DDI_HSHADERRESOURCEVIEW shader_resource_view, D3D10DDI_HRTSHADERRESOURCEVIEW rt_shader_resource_view)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(create_shader_resource_view);
    UNREFERENCED_PARAMETER(shader_resource_view);
    UNREFERENCED_PARAMETER(rt_shader_resource_view);
}

void APIENTRY umd_device::destroy_shader_resource_view(D3D10DDI_HDEVICE device,
        D3D10DDI_HSHADERRESOURCEVIEW shader_resource_view)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(shader_resource_view);
}
