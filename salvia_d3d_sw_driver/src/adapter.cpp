#define INITGUID
#include <salvia_d3d_sw_driver/include/common.h>

#include <eflib/include/platform/typedefs.h>

#include <salvia_d3d_sw_driver/include/adapter.h>
#include <salvia_d3d_sw_driver/include/device.h>

HRESULT APIENTRY OpenAdapter10(D3D10DDIARG_OPENADAPTER* pOpenData)
{
    HRESULT hr = S_OK;

    umd_adapter* adapter = new (std::nothrow) umd_adapter;
    if (nullptr == adapter)
    {
        return E_OUTOFMEMORY;
    }

    hr = adapter->open_adapter_10(pOpenData);
    if (FAILED(hr))
    {
        delete adapter;
        return hr;
    }

    return hr;
}

HRESULT APIENTRY OpenAdapter10_2(D3D10DDIARG_OPENADAPTER* pOpenData)
{
    HRESULT hr = S_OK;

    umd_adapter* adapter = new (std::nothrow) umd_adapter;
    if (nullptr == adapter)
    {
        return E_OUTOFMEMORY;
    }

    hr = adapter->open_adapter_10_2(pOpenData);
    if (FAILED(hr))
    {
        delete adapter;
        return hr;
    }

    return hr;
}


umd_adapter::umd_adapter()
{
}

HRESULT umd_adapter::open_adapter_10(D3D10DDIARG_OPENADAPTER* args)
{
    rt_adapter_ = args->hRTAdapter;
    interface_ = args->Interface;
    version_ = args->Version;
    adapter_callbacks_ = args->pAdapterCallbacks;

    const D3D10DDI_ADAPTERFUNCS AdapterFuncs =
    {
        calc_private_device_size,
        create_device,
        close_adapter
    };
    *args->pAdapterFuncs = AdapterFuncs;
    args->hAdapter.pDrvPrivate = this;

    return S_OK;
}

HRESULT umd_adapter::open_adapter_10_2(D3D10DDIARG_OPENADAPTER* args)
{
    rt_adapter_ = args->hRTAdapter;
    interface_ = args->Interface;
    version_ = args->Version;
    adapter_callbacks_ = args->pAdapterCallbacks;

    const D3D10_2DDI_ADAPTERFUNCS AdapterFuncs =
    {
        calc_private_device_size,
        create_device,
        close_adapter,
        get_supported_versions,
        get_caps
    };
    *args->pAdapterFuncs_2 = AdapterFuncs;
    args->hAdapter.pDrvPrivate = this;

    return S_OK;
}

SIZE_T APIENTRY umd_adapter::calc_private_device_size(D3D10DDI_HADAPTER ddi_adapter, const D3D10DDIARG_CALCPRIVATEDEVICESIZE* args)
{
    UNREFERENCED_PARAMETER(ddi_adapter);
    UNREFERENCED_PARAMETER(args);

    return sizeof(umd_device);
}

HRESULT APIENTRY umd_adapter::create_device(D3D10DDI_HADAPTER ddi_adapter, D3D10DDIARG_CREATEDEVICE* args)
{
    umd_adapter* adpter = static_cast<umd_adapter*>(ddi_adapter.pDrvPrivate);
    new (args->hDrvDevice.pDrvPrivate) umd_device(adpter, args);
    return S_OK;
}

HRESULT APIENTRY umd_adapter::close_adapter(D3D10DDI_HADAPTER ddi_adapter)
{
    umd_adapter* adpter = static_cast<umd_adapter*>(ddi_adapter.pDrvPrivate);
    delete adpter;
    return S_OK;
}

HRESULT APIENTRY umd_adapter::get_supported_versions(D3D10DDI_HADAPTER ddi_adapter, UINT32* entries, UINT64* ddi_versions)
{
    UNREFERENCED_PARAMETER(ddi_adapter);

    static const UINT64 supported_versions[] =
    {
        D3D11_0_DDI_SUPPORTED,
        D3D11_0_vista_DDI_SUPPORTED,
        D3D11_0_7_DDI_SUPPORTED
        // TODO: D3D 11.1, 11.2?
    };
    uint32_t num_supported_versions = sizeof(supported_versions) / sizeof(supported_versions[0]);

    HRESULT hr = S_OK;
    if (ddi_versions != nullptr)
    {
        if (*entries < num_supported_versions)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            *entries = num_supported_versions;
            memcpy(ddi_versions, supported_versions, sizeof(supported_versions[0]) * num_supported_versions);
        }
    }
    else
    {
        *entries = num_supported_versions;
    }

    return hr;
}

HRESULT APIENTRY umd_adapter::get_caps(D3D10DDI_HADAPTER ddi_adapter, const D3D10_2DDIARG_GETCAPS* caps)
{
    UNREFERENCED_PARAMETER(ddi_adapter);

    HRESULT hr = S_OK;

    switch (caps->Type)
    {
    case D3D11DDICAPS_THREADING:
        if (caps->DataSize != sizeof(D3D11DDI_THREADING_CAPS))
        {
            hr = E_UNEXPECTED;
        }
        else
        {
            D3D11DDI_THREADING_CAPS* data = static_cast<D3D11DDI_THREADING_CAPS*>(caps->pData);
            data->Caps = 0;
        }
        break;

    case D3D11DDICAPS_SHADER:
        if (caps->DataSize != sizeof(D3D11DDI_SHADER_CAPS))
        {
            hr = E_UNEXPECTED;
        }
        else
        {
            D3D11DDI_SHADER_CAPS* data = static_cast<D3D11DDI_SHADER_CAPS*>(caps->pData);
            data->Caps = 0;
        }
        break;

    case D3D11DDICAPS_3DPIPELINESUPPORT:
        if (caps->DataSize != sizeof(D3D11DDI_3DPIPELINESUPPORT_CAPS))
        {
            hr = E_UNEXPECTED;
        }
        else
        {
            D3D11DDI_3DPIPELINESUPPORT_CAPS* data = static_cast<D3D11DDI_3DPIPELINESUPPORT_CAPS*>(caps->pData);
            data->Caps = D3D11DDI_ENCODE_3DPIPELINESUPPORT_CAP(D3D11DDI_3DPIPELINELEVEL_10_0)
                | D3D11DDI_ENCODE_3DPIPELINESUPPORT_CAP(D3D11DDI_3DPIPELINELEVEL_10_1)
                | D3D11DDI_ENCODE_3DPIPELINESUPPORT_CAP(D3D11DDI_3DPIPELINELEVEL_11_0)
                | D3D11DDI_ENCODE_3DPIPELINESUPPORT_CAP(D3D11_1DDI_3DPIPELINELEVEL_11_1);
        }
        break;

#if D3D11DDI_MINOR_HEADER_VERSION >= 3
    case D3D11_1DDICAPS_D3D11_OPTIONS:
        if (caps->DataSize != sizeof(D3D11_1DDI_D3D11_OPTIONS_DATA))
        {
            hr = E_UNEXPECTED;
        }
        else
        {
            D3D11_1DDI_D3D11_OPTIONS_DATA* data = static_cast<D3D11_1DDI_D3D11_OPTIONS_DATA*>(caps->pData);
            data->OutputMergerLogicOp = FALSE;
            data->AssignDebugBinarySupport = FALSE;
        }
        break;

    case D3D11_1DDICAPS_ARCHITECTURE_INFO:
        if (caps->DataSize != sizeof(D3DDDICAPS_ARCHITECTURE_INFO))
        {
            hr = E_UNEXPECTED;
        }
        else
        {
            D3DDDICAPS_ARCHITECTURE_INFO* data = static_cast<D3DDDICAPS_ARCHITECTURE_INFO*>(caps->pData);
            data->TileBasedDeferredRenderer = FALSE;
        }
        break;

    case D3D11_1DDICAPS_SHADER_MIN_PRECISION_SUPPORT:
        if (caps->DataSize != sizeof(D3DDDICAPS_SHADER_MIN_PRECISION_SUPPORT))
        {
            hr = E_UNEXPECTED;
        }
        else
        {
            D3DDDICAPS_SHADER_MIN_PRECISION_SUPPORT* data = static_cast<D3DDDICAPS_SHADER_MIN_PRECISION_SUPPORT*>(caps->pData);
            data->VertexShaderMinPrecision = D3DDDICAPS_SHADER_MIN_PRECISION_16_BIT;
            data->PixelShaderMinPrecision = D3DDDICAPS_SHADER_MIN_PRECISION_16_BIT;
        }
        break;
#endif

#if D3D11DDI_MINOR_HEADER_VERSION >= 4
    case D3DWDDM1_3DDICAPS_D3D11_OPTIONS1:
        if (caps->DataSize != sizeof(D3DWDDM1_3DDI_D3D11_OPTIONS_DATA1))
        {
            hr = E_UNEXPECTED;
        }
        else
        {
            D3DWDDM1_3DDI_D3D11_OPTIONS_DATA1* data = static_cast<D3DWDDM1_3DDI_D3D11_OPTIONS_DATA1*>(caps->pData);
            data->TiledResourcesSupportFlags = FALSE;
        }
        break;

    case D3DWDDM1_3DDICAPS_MARKER:
        if (caps->DataSize != sizeof(UINT))
        {
            hr = E_UNEXPECTED;
        }
        else
        {
            UINT* data = static_cast<UINT*>(caps->pData);
            *data = 0;
            hr = E_FAIL;
        }
        break;
#endif

    default:
        hr = E_NOTIMPL;
        break;
    }

    return hr;
}
