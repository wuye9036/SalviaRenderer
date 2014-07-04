#define INITGUID
#include "common.h"

#include <salvia_d3d_sw_driver/include/display.h>
#include <salvia_d3d_sw_driver/include/kmd_adapter.h>
#include <salvia_d3d_sw_driver/include/kmd_device.h>
#include <salvia_d3d_sw_driver/include/kmd_context.h>

boost::mutex g_km_mutex;
std::list<kmd_adapter*> g_kmd_adapters;
std::list<kmd_device*> g_kmd_devices;
std::list<kmd_context*> g_kmd_contexts;

void km_destroy()
{
	boost::mutex::scoped_lock lock(g_km_mutex);

	for (auto iter = g_kmd_adapters.begin(); iter != g_kmd_adapters.end(); ++ iter)
	{
		delete *iter;
	}
	for (auto iter = g_kmd_devices.begin(); iter != g_kmd_devices.end(); ++ iter)
	{
		delete *iter;
	}
	for (auto iter = g_kmd_contexts.begin(); iter != g_kmd_contexts.end(); ++ iter)
	{
		delete *iter;
	}
}


NTSTATUS APIENTRY D3DKMTCreateAllocation(D3DKMT_CREATEALLOCATION* pData)
{
	UNREFERENCED_PARAMETER(pData);
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS APIENTRY D3DKMTQueryResourceInfo(D3DKMT_QUERYRESOURCEINFO* pData)
{
	UNREFERENCED_PARAMETER(pData);
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS APIENTRY D3DKMTOpenResource(D3DKMT_OPENRESOURCE* pData)
{
	UNREFERENCED_PARAMETER(pData);
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS APIENTRY D3DKMTDestroyAllocation(const D3DKMT_DESTROYALLOCATION* pData)
{
	UNREFERENCED_PARAMETER(pData);
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS APIENTRY D3DKMTSetAllocationPriority(const D3DKMT_SETALLOCATIONPRIORITY* pData)
{
	UNREFERENCED_PARAMETER(pData);
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS APIENTRY D3DKMTQueryAllocationResidency(const D3DKMT_QUERYALLOCATIONRESIDENCY* pData)
{
	UNREFERENCED_PARAMETER(pData);
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS APIENTRY D3DKMTCreateDevice(D3DKMT_CREATEDEVICE* pData)
{
	NTSTATUS status;

	auto iter = std::find(g_kmd_adapters.begin(), g_kmd_adapters.end(), reinterpret_cast<kmd_adapter*>(pData->hAdapter));
	if (iter == g_kmd_adapters.end())
	{
		status = STATUS_INVALID_PARAMETER;
	}
	else
	{
		status = (*iter)->create_device(pData);
	}

	return status;
}

NTSTATUS APIENTRY D3DKMTDestroyDevice(const D3DKMT_DESTROYDEVICE* pData)
{
	NTSTATUS status;

	auto iter = std::find(g_kmd_devices.begin(), g_kmd_devices.end(), reinterpret_cast<kmd_device*>(pData->hDevice));
	if (iter == g_kmd_devices.end())
	{
		status = STATUS_INVALID_PARAMETER;
	}
	else
	{
		delete *iter;
		g_kmd_devices.erase(iter);
		status = STATUS_SUCCESS;
	}

	return status;
}

NTSTATUS APIENTRY D3DKMTLock(D3DKMT_LOCK* pData)
{
	UNREFERENCED_PARAMETER(pData);
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS APIENTRY D3DKMTUnlock(const D3DKMT_UNLOCK* pData)
{
	UNREFERENCED_PARAMETER(pData);
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS APIENTRY D3DKMTGetDisplayModeList(D3DKMT_GETDISPLAYMODELIST* pData)
{
	UNREFERENCED_PARAMETER(pData);
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS APIENTRY D3DKMTSetDisplayMode(const D3DKMT_SETDISPLAYMODE* pData)
{
	UNREFERENCED_PARAMETER(pData);
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS APIENTRY D3DKMTGetMultisampleMethodList(D3DKMT_GETMULTISAMPLEMETHODLIST* pData)
{
	UNREFERENCED_PARAMETER(pData);
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS APIENTRY D3DKMTPresent(const D3DKMT_PRESENT* pData)
{
	UNREFERENCED_PARAMETER(pData);
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS APIENTRY D3DKMTRender(D3DKMT_RENDER* pData)
{
	UNREFERENCED_PARAMETER(pData);
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS APIENTRY D3DKMTGetRuntimeData(const D3DKMT_GETRUNTIMEDATA* pData)
{
	UNREFERENCED_PARAMETER(pData);
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS APIENTRY D3DKMTQueryAdapterInfo(const D3DKMT_QUERYADAPTERINFO* pData)
{
	NTSTATUS status;

	auto iter = std::find(g_kmd_adapters.begin(), g_kmd_adapters.end(), reinterpret_cast<kmd_adapter*>(pData->hAdapter));
	if (iter == g_kmd_adapters.end())
	{
		status = STATUS_INVALID_PARAMETER;
	}
	else
	{
		status = (*iter)->query_adapter_info(pData);
	}

	return status;
}

NTSTATUS APIENTRY D3DKMTOpenAdapterFromGdiDisplayName(D3DKMT_OPENADAPTERFROMGDIDISPLAYNAME* pData)
{
	NTSTATUS status;

	boost::shared_ptr<display_mgr> disp_mgr = display_mgr::instance();
	kmd_adapter* adapter = new (std::nothrow) kmd_adapter;
	if (nullptr == adapter)
	{
		status = STATUS_NO_MEMORY;
	}
	else
	{
		status = adapter->create(disp_mgr, pData);
		if (STATUS_SUCCESS == status)
		{
			g_kmd_adapters.push_back(adapter);
		}
		else
		{
			delete adapter;
		}
	}

	return status;
}

NTSTATUS APIENTRY D3DKMTOpenAdapterFromDeviceName(D3DKMT_OPENADAPTERFROMDEVICENAME* pData)
{
	boost::shared_ptr<display_mgr> disp_mgr = display_mgr::instance();

	D3DKMT_OPENADAPTERFROMGDIDISPLAYNAME adapter_from_gdi;
	bool remoting = false;
	for (uint32_t index = 0; index < disp_mgr->num_displays(); ++ index)
	{
		display* disp = disp_mgr->get_display(index);
		assert(disp != nullptr);

		uint32_t flags = disp->state_flags();
		if ((flags & DISPLAY_DEVICE_REMOTE) && (flags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP)
			&& (flags & DISPLAY_DEVICE_PRIMARY_DEVICE))
		{
			memcpy(adapter_from_gdi.DeviceName, disp->name().c_str(), disp->name().size() * sizeof(wchar_t));
			remoting = true;
		}
	}

	NTSTATUS status;

	kmd_adapter* adapter = new (std::nothrow) kmd_adapter;
	if (nullptr == adapter)
	{
		status = STATUS_NO_MEMORY;
	}
	else
	{
		if (!remoting)
		{
			status = adapter->create(&pData->hAdapter, &pData->AdapterLuid);
		}
		else
		{
			status = adapter->create(disp_mgr, &adapter_from_gdi);

			pData->hAdapter = adapter_from_gdi.hAdapter;
			pData->AdapterLuid = adapter_from_gdi.AdapterLuid;
		}
	}

	return status;
}

NTSTATUS APIENTRY D3DKMTCloseAdapter(const D3DKMT_CLOSEADAPTER* pData)
{
	NTSTATUS status;

	boost::mutex::scoped_lock lock(g_km_mutex);

	auto iter = std::find(g_kmd_adapters.begin(), g_kmd_adapters.end(), reinterpret_cast<kmd_adapter*>(pData->hAdapter));
	if (iter == g_kmd_adapters.end())
	{
		status = STATUS_INVALID_PARAMETER;
	}
	else
	{
		delete *iter;
		g_kmd_adapters.erase(iter);
		status = STATUS_SUCCESS;
	}

	return status;
}

NTSTATUS APIENTRY D3DKMTGetSharedPrimaryHandle(D3DKMT_GETSHAREDPRIMARYHANDLE* pData)
{
	UNREFERENCED_PARAMETER(pData);
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS APIENTRY D3DKMTEscape(const D3DKMT_ESCAPE* pData)
{
	UNREFERENCED_PARAMETER(pData);
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS APIENTRY D3DKMTSetVidPnSourceOwner(const D3DKMT_SETVIDPNSOURCEOWNER* pData)
{
	UNREFERENCED_PARAMETER(pData);
	return STATUS_NOT_IMPLEMENTED;
}


NTSTATUS APIENTRY D3DKMTWaitForVerticalBlankEvent(const D3DKMT_WAITFORVERTICALBLANKEVENT* pData)
{
	UNREFERENCED_PARAMETER(pData);
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS APIENTRY D3DKMTSetGammaRamp(const D3DKMT_SETGAMMARAMP* pData)
{
	UNREFERENCED_PARAMETER(pData);
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS APIENTRY D3DKMTGetDeviceState(D3DKMT_GETDEVICESTATE* pData)
{
	UNREFERENCED_PARAMETER(pData);
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS APIENTRY D3DKMTSetContextSchedulingPriority(const D3DKMT_SETCONTEXTSCHEDULINGPRIORITY* pData)
{
	UNREFERENCED_PARAMETER(pData);
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS APIENTRY D3DKMTGetContextSchedulingPriority(D3DKMT_GETCONTEXTSCHEDULINGPRIORITY* pData)
{
	UNREFERENCED_PARAMETER(pData);
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS APIENTRY D3DKMTCreateContext(D3DKMT_CREATECONTEXT* pData)
{
	NTSTATUS status;

	auto iter = std::find(g_kmd_devices.begin(), g_kmd_devices.end(), reinterpret_cast<kmd_device*>(pData->hDevice));
	if (iter == g_kmd_devices.end())
	{
		status = STATUS_INVALID_PARAMETER;
	}
	else
	{
		status = (*iter)->create_context(pData);
	}

	return status;
}

NTSTATUS APIENTRY D3DKMTDestroyContext(const D3DKMT_DESTROYCONTEXT* pData)
{
	NTSTATUS status;

	auto iter = std::find(g_kmd_contexts.begin(), g_kmd_contexts.end(), reinterpret_cast<kmd_context*>(pData->hContext));
	if (iter == g_kmd_contexts.end())
	{
		status = STATUS_INVALID_PARAMETER;
	}
	else
	{
		delete *iter;
		g_kmd_contexts.erase(iter);
		status = STATUS_SUCCESS;
	}

	return status;
}

NTSTATUS APIENTRY D3DKMTCreateSynchronizationObject(D3DKMT_CREATESYNCHRONIZATIONOBJECT* pData)
{
	UNREFERENCED_PARAMETER(pData);
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS APIENTRY D3DKMTDestroySynchronizationObject(const D3DKMT_DESTROYSYNCHRONIZATIONOBJECT* pData)
{
	UNREFERENCED_PARAMETER(pData);
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS APIENTRY D3DKMTWaitForSynchronizationObject(const D3DKMT_WAITFORSYNCHRONIZATIONOBJECT* pData)
{
	UNREFERENCED_PARAMETER(pData);
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS APIENTRY D3DKMTSignalSynchronizationObject(const D3DKMT_SIGNALSYNCHRONIZATIONOBJECT* pData)
{
	UNREFERENCED_PARAMETER(pData);
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS APIENTRY D3DKMTSetDisplayPrivateDriverFormat(const D3DKMT_SETDISPLAYPRIVATEDRIVERFORMAT* pData)
{
	UNREFERENCED_PARAMETER(pData);
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS APIENTRY D3DKMTSignalSynchronizationObject2(const D3DKMT_SIGNALSYNCHRONIZATIONOBJECT2* pData)
{
	UNREFERENCED_PARAMETER(pData);
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS APIENTRY D3DKMTWaitForSynchronizationObject2(const D3DKMT_WAITFORSYNCHRONIZATIONOBJECT2* pData)
{
	UNREFERENCED_PARAMETER(pData);
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS APIENTRY D3DKMTOpenSynchronizationObject(D3DKMT_OPENSYNCHRONIZATIONOBJECT* pData)
{
	UNREFERENCED_PARAMETER(pData);
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS APIENTRY D3DKMTCreateSynchronizationObject2(D3DKMT_CREATESYNCHRONIZATIONOBJECT2* pData)
{
	UNREFERENCED_PARAMETER(pData);
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS APIENTRY D3DKMTReleaseKeyedMutex(D3DKMT_RELEASEKEYEDMUTEX* pData)
{
	UNREFERENCED_PARAMETER(pData);
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS APIENTRY D3DKMTAcquireKeyedMutex(D3DKMT_ACQUIREKEYEDMUTEX* pData)
{
	UNREFERENCED_PARAMETER(pData);
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS APIENTRY D3DKMTDestroyKeyedMutex(const D3DKMT_DESTROYKEYEDMUTEX* pData)
{
	UNREFERENCED_PARAMETER(pData);
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS APIENTRY D3DKMTOpenKeyedMutex(D3DKMT_OPENKEYEDMUTEX* pData)
{
	UNREFERENCED_PARAMETER(pData);
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS APIENTRY D3DKMTCreateKeyedMutex(D3DKMT_CREATEKEYEDMUTEX* pData)
{
	UNREFERENCED_PARAMETER(pData);
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS APIENTRY D3DKMTOpenResource2(D3DKMT_OPENRESOURCE* pData)
{
	UNREFERENCED_PARAMETER(pData);
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS APIENTRY D3DKMTCreateAllocation2(D3DKMT_CREATEALLOCATION* pData)
{
	UNREFERENCED_PARAMETER(pData);
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS APIENTRY D3DKMTConfigureSharedResource(const D3DKMT_CONFIGURESHAREDRESOURCE* pData)
{
	UNREFERENCED_PARAMETER(pData);
	return STATUS_NOT_IMPLEMENTED;
}
