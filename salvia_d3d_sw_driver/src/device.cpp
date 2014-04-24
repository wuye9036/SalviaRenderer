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
        d3d11_device_funcs_->pfnDynamicIABufferMapNoOverwrite = resource_map;
        d3d11_device_funcs_->pfnDynamicIABufferUnmap = resource_unmap;
        d3d11_device_funcs_->pfnDynamicConstantBufferMapDiscard = resource_map;
        d3d11_device_funcs_->pfnDynamicIABufferMapDiscard = resource_map;
        d3d11_device_funcs_->pfnDynamicConstantBufferUnmap = resource_unmap;
        d3d11_device_funcs_->pfnPsSetConstantBuffers = ps_set_constant_buffers;
        d3d11_device_funcs_->pfnIaSetInputLayout = ia_set_input_layout;
        d3d11_device_funcs_->pfnIaSetVertexBuffers = ia_set_vertex_buffers;
        d3d11_device_funcs_->pfnIaSetIndexBuffer = ia_set_index_buffer;

        // MIDDLE-FREQUENCY
        d3d11_device_funcs_->pfnDrawIndexedInstanced = nullptr;
        d3d11_device_funcs_->pfnDrawInstanced = nullptr;
        d3d11_device_funcs_->pfnDynamicResourceMapDiscard = nullptr;
        d3d11_device_funcs_->pfnDynamicResourceUnmap = nullptr;
        d3d11_device_funcs_->pfnGsSetConstantBuffers = nullptr;
        d3d11_device_funcs_->pfnGsSetShader = nullptr;
        d3d11_device_funcs_->pfnIaSetTopology = nullptr;
        d3d11_device_funcs_->pfnStagingResourceMap = nullptr;
        d3d11_device_funcs_->pfnStagingResourceUnmap = nullptr;
        d3d11_device_funcs_->pfnVsSetShaderResources = nullptr;
        d3d11_device_funcs_->pfnVsSetSamplers = nullptr;
        d3d11_device_funcs_->pfnGsSetShaderResources = nullptr;
        d3d11_device_funcs_->pfnGsSetSamplers = nullptr;
        d3d11_device_funcs_->pfnSetRenderTargets = nullptr;
        d3d11_device_funcs_->pfnShaderResourceViewReadAfterWriteHazard = nullptr;
        d3d11_device_funcs_->pfnResourceReadAfterWriteHazard = nullptr;
        d3d11_device_funcs_->pfnSetBlendState = nullptr;
        d3d11_device_funcs_->pfnSetDepthStencilState = nullptr;
        d3d11_device_funcs_->pfnSetRasterizerState = nullptr;
        d3d11_device_funcs_->pfnQueryEnd = nullptr;
        d3d11_device_funcs_->pfnQueryBegin = nullptr;
        d3d11_device_funcs_->pfnResourceCopyRegion = nullptr;
        d3d11_device_funcs_->pfnResourceUpdateSubresourceUP = nullptr;
        d3d11_device_funcs_->pfnSoSetTargets = nullptr;
        d3d11_device_funcs_->pfnDrawAuto = nullptr;
        d3d11_device_funcs_->pfnSetViewports = nullptr;
        d3d11_device_funcs_->pfnSetScissorRects = nullptr;
        d3d11_device_funcs_->pfnClearRenderTargetView = nullptr;
        d3d11_device_funcs_->pfnClearDepthStencilView = nullptr;
        d3d11_device_funcs_->pfnSetPredication = nullptr;
        d3d11_device_funcs_->pfnQueryGetData = nullptr;
        d3d11_device_funcs_->pfnFlush = nullptr;
        d3d11_device_funcs_->pfnGenMips = nullptr;
        d3d11_device_funcs_->pfnResourceCopy = nullptr;
        d3d11_device_funcs_->pfnResourceResolveSubresource = nullptr;

        // Infrequent paths
        d3d11_device_funcs_->pfnResourceMap = nullptr;
        d3d11_device_funcs_->pfnResourceUnmap = nullptr;
        d3d11_device_funcs_->pfnResourceIsStagingBusy = nullptr;
        d3d11_device_funcs_->pfnRelocateDeviceFuncs = nullptr;
        d3d11_device_funcs_->pfnCalcPrivateResourceSize = nullptr;
        d3d11_device_funcs_->pfnCalcPrivateOpenedResourceSize = nullptr;
        d3d11_device_funcs_->pfnCreateResource = nullptr;
        d3d11_device_funcs_->pfnOpenResource = nullptr;
        d3d11_device_funcs_->pfnDestroyResource = nullptr;
        d3d11_device_funcs_->pfnCalcPrivateShaderResourceViewSize = nullptr;
        d3d11_device_funcs_->pfnCreateShaderResourceView = nullptr;
        d3d11_device_funcs_->pfnDestroyShaderResourceView = nullptr;
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
