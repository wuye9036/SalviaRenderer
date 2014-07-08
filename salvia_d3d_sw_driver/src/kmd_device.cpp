#define INITGUID
#include <salvia_d3d_sw_driver/include/common.h>

#include <eflib/include/platform/typedefs.h>

#include <salvia_d3d_sw_driver/include/kmd_adapter.h>
#include <salvia_d3d_sw_driver/include/kmd_device.h>
#include <salvia_d3d_sw_driver/include/kmd_context.h>

NTSTATUS kmd_device::create(D3DKMT_CREATEDEVICE* cd)
{
	boost::mutex::scoped_lock lock(g_km_mutex);

	device_ = reinterpret_cast<D3DKMT_HANDLE>(this);
	cd->hDevice = device_;

	return STATUS_SUCCESS;
}

NTSTATUS kmd_device::create_context(D3DKMT_CREATECONTEXT* cd)
{
	NTSTATUS status;

	if ((cd->NodeOrdinal != 0) || (cd->EngineAffinity != 0) || (cd->ClientHint != D3DKMT_CLIENTHINT_DX10))
	{
		status = STATUS_INVALID_PARAMETER;
	}
	else
	{
		kmd_context* context = new (std::nothrow) kmd_context;
		if (nullptr == context)
		{
			status = STATUS_NO_MEMORY;
		}
		else
		{
			status = context->create(cd, this);
			if (STATUS_SUCCESS == status)
			{
				g_kmd_contexts.push_back(context);
			}
			else
			{
				delete context;
			}			
		}
	}

	return status;
}

NTSTATUS kmd_device::escape(const D3DKMT_ESCAPE* esc)
{
	NTSTATUS status;

	if ((nullptr == esc->pPrivateDriverData) || (esc->PrivateDriverDataSize != sizeof(void*)))
	{
		status = STATUS_INVALID_PARAMETER;
	}
	else
	{
		umd_device_ = *static_cast<umd_device**>(esc->pPrivateDriverData);
		status = STATUS_SUCCESS;
	}

	return status;
}
