#pragma once

class umd_adapter
{
public:
	umd_adapter();

	HRESULT open_adapter_10(D3D10DDIARG_OPENADAPTER* args);
	HRESULT open_adapter_10_2(D3D10DDIARG_OPENADAPTER* args);

	D3D10DDI_HRTADAPTER rt_adapter() const
	{
		return rt_adapter_;
	}

private:
	static SIZE_T APIENTRY calc_private_device_size(D3D10DDI_HADAPTER ddi_adapter, const D3D10DDIARG_CALCPRIVATEDEVICESIZE* args);
	static HRESULT APIENTRY create_device(D3D10DDI_HADAPTER ddi_adapter, D3D10DDIARG_CREATEDEVICE* args);
	static HRESULT APIENTRY close_adapter(D3D10DDI_HADAPTER ddi_adapter);
	static HRESULT APIENTRY get_supported_versions(D3D10DDI_HADAPTER ddi_adapter, UINT32* entries, UINT64* ddi_versions);
	static HRESULT APIENTRY get_caps(D3D10DDI_HADAPTER ddi_adapter, const D3D10_2DDIARG_GETCAPS* caps);

private:
	D3D10DDI_HRTADAPTER rt_adapter_;
	UINT interface_;
	UINT version_;
	const D3DDDI_ADAPTERCALLBACKS* adapter_callbacks_;
};
