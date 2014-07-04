#define INITGUID
#include <salvia_d3d_sw_driver/include/common.h>

#include <eflib/include/platform/typedefs.h>

#include <cassert>

#include <salvia_d3d_sw_driver/include/kmd_adapter.h>
#include <salvia_d3d_sw_driver/include/kmd_device.h>
#include <salvia_d3d_sw_driver/include/display.h>

NTSTATUS kmd_adapter::create(const boost::shared_ptr<display_mgr>& disp_mgr, D3DKMT_OPENADAPTERFROMGDIDISPLAYNAME* adapter_from_name)
{
	assert(disp_mgr);

	boost::mutex::scoped_lock lock(g_km_mutex);
	{
		for (uint32_t i = 0; i < disp_mgr->num_displays(); ++ i)
		{
			const display* disp = disp_mgr->get_display(i);
			if (!disp)
			{
				return STATUS_UNSUCCESSFUL;
			}

			if (disp->name() == adapter_from_name->DeviceName)
			{
				adapter_from_name->VidPnSourceId = i;
				break;
			}
		}

		adapter_ = reinterpret_cast<D3DKMT_HANDLE>(this);
		adapter_from_name->hAdapter = adapter_;

		ZeroMemory(&adapter_from_name->AdapterLuid, sizeof(adapter_from_name->AdapterLuid));
		adapter_from_name->AdapterLuid.HighPart = static_cast<LONG>(g_kmd_devices.size() - 1);
	}

	return STATUS_SUCCESS;
}

NTSTATUS kmd_adapter::create(D3DKMT_HANDLE* adapter, LUID* adapter_luid)
{
	boost::mutex::scoped_lock lock(g_km_mutex);
	{
		adapter_ = reinterpret_cast<D3DKMT_HANDLE>(this);
		*adapter = adapter_;

		ZeroMemory(adapter_luid, sizeof(*adapter_luid));
		adapter_luid->HighPart = static_cast<LONG>(g_kmd_devices.size() - 1);
	}

	return STATUS_SUCCESS;
}

NTSTATUS kmd_adapter::query_adapter_info(const D3DKMT_QUERYADAPTERINFO* query_ai)
{
	switch (query_ai->Type)
	{
	case KMTQAITYPE_UMDRIVERPRIVATE:
		// TODO
		break;

	case KMTQAITYPE_UMDRIVERNAME:
		{
			D3DKMT_UMDFILENAMEINFO* umd_fn = static_cast<D3DKMT_UMDFILENAMEINFO*>(query_ai->pPrivateDriverData);
			if (query_ai->PrivateDriverDataSize != sizeof(*umd_fn))
			{
				return STATUS_INVALID_PARAMETER;
			}

			if ((umd_fn->Version != KMTUMDVERSION_DX10) && (umd_fn->Version != KMTUMDVERSION_DX11))
			{
				return STATUS_INVALID_PARAMETER;
			}

			if (0 == GetModuleFileNameW(g_dll, umd_fn->UmdFileName, ARRAYSIZE(umd_fn->UmdFileName)))
			{
				return STATUS_UNSUCCESSFUL;
			}
		}
		break;

	case KMTQAITYPE_GETSEGMENTSIZE:
		{
			MEMORYSTATUSEX mem_status;
			mem_status.dwLength = sizeof(mem_status);
			GlobalMemoryStatusEx(&mem_status);

			D3DKMT_SEGMENTSIZEINFO* psi = reinterpret_cast<D3DKMT_SEGMENTSIZEINFO*>(query_ai->pPrivateDriverData);
			psi->DedicatedVideoMemorySize = 0;
			psi->DedicatedSystemMemorySize = 0;
			psi->SharedSystemMemorySize = std::max((mem_status.ullTotalPhys - 512) / 2, 64ULL);
		}
		break;

	case KMTQAITYPE_CHECKDRIVERUPDATESTATUS:
		{
			BOOL* pbDriverUpdateInProgress = reinterpret_cast<BOOL*>(query_ai->pPrivateDriverData);
			*pbDriverUpdateInProgress = FALSE;
		}
		break;

	case KMTQAITYPE_DIRECTFLIP_SUPPORT:
		{
			D3DKMT_DIRECTFLIP_SUPPORT* direct_flip_support = reinterpret_cast<D3DKMT_DIRECTFLIP_SUPPORT*>(query_ai->pPrivateDriverData);
			direct_flip_support->Supported = FALSE;
		}
		break;

	case KMTQAITYPE_FLIPQUEUEINFO:
		{
			D3DKMT_FLIPQUEUEINFO* info = reinterpret_cast<D3DKMT_FLIPQUEUEINFO*>(query_ai->pPrivateDriverData);
			info->MaxHardwareFlipQueueLength = 0;
			info->MaxSoftwareFlipQueueLength = 0;
			info->FlipFlags.FlipInterval = 0;
		}
		break;

	case KMTQAITYPE_ADAPTERTYPE:
		{
			D3DKMT_ADAPTERTYPE* type = reinterpret_cast<D3DKMT_ADAPTERTYPE*>(query_ai->pPrivateDriverData);
			type->RenderSupported  = true;
			type->DisplaySupported = true;
			type->SoftwareDevice = false;
			type->PostDevice = false;
			type->Reserved = false;
		}
		break;

	case KMTQAITYPE_WDDM_1_2_CAPS:
		{
			D3DKMT_WDDM_1_2_CAPS* caps = reinterpret_cast<D3DKMT_WDDM_1_2_CAPS*>(query_ai->pPrivateDriverData);
			caps->PreemptionCaps.GraphicsPreemptionGranularity = D3DKMDT_GRAPHICS_PREEMPTION_NONE;
			caps->PreemptionCaps.ComputePreemptionGranularity = D3DKMDT_COMPUTE_PREEMPTION_NONE;
			caps->SupportNonVGA = true;
			caps->SupportSmoothRotation = false;
			caps->SupportPerEngineTDR = false;
			caps->SupportKernelModeCommandBuffer = false;
			caps->SupportCCD = false;
			caps->SupportSoftwareDeviceBitmaps = false;
			caps->SupportGammaRamp = false;
			caps->SupportHWCursor = false;
			caps->SupportHWVSync = true;
			caps->SupportSurpriseRemovalInHibernation = false;
		}
		break;

	case KMTQAITYPE_DRIVERVERSION:
		{
			D3DKMT_DRIVERVERSION* version = reinterpret_cast<D3DKMT_DRIVERVERSION*>(query_ai->pPrivateDriverData);
			*version = KMT_DRIVERVERSION_WDDM_1_0;
		}
		break;

	default:
		return STATUS_INVALID_PARAMETER;
	}

	return STATUS_SUCCESS;
}

NTSTATUS kmd_adapter::create_device(D3DKMT_CREATEDEVICE* cd)
{
	NTSTATUS status;

	kmd_device* device = new (std::nothrow) kmd_device;
	if (nullptr == device)
	{
		status = STATUS_NO_MEMORY;
	}
	else
	{
		status = device->create(cd);
		if (STATUS_SUCCESS == status)
		{
			g_kmd_devices.push_back(device);
		}
		else
		{
			delete device;
		}		
	}

	return status;
}
