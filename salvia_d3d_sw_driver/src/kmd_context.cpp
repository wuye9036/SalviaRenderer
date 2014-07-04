#define INITGUID
#include <salvia_d3d_sw_driver/include/common.h>

#include <eflib/include/platform/typedefs.h>

#include <salvia_d3d_sw_driver/include/kmd_device.h>
#include <salvia_d3d_sw_driver/include/kmd_context.h>

NTSTATUS kmd_context::create(D3DKMT_CREATECONTEXT* cc)
{
	NTSTATUS status;

	if ((cc->Flags.NullRendering != 0) || (cc->Flags.Reserved != 0))
	{
		status = STATUS_INVALID_PARAMETER;
	}
	else
	{
		boost::mutex::scoped_lock lock(g_km_mutex);

		// TODO
		cc->CommandBufferSize = 32 * 1024;
		cc->AllocationListSize = 3 * 1024;
		cc->PatchLocationListSize = 3 * 1024;
		cc->pCommandBuffer = new (std::nothrow) BYTE[cc->CommandBufferSize];
		cc->pAllocationList = new (std::nothrow) D3DDDI_ALLOCATIONLIST[cc->AllocationListSize];
		cc->pPatchLocationList = new (std::nothrow) D3DDDI_PATCHLOCATIONLIST[cc->PatchLocationListSize];
		ZeroMemory(cc->pAllocationList, cc->AllocationListSize * sizeof(D3DDDI_ALLOCATIONLIST));
		ZeroMemory(cc->pPatchLocationList, cc->PatchLocationListSize * sizeof(D3DDDI_PATCHLOCATIONLIST));

		context_ = reinterpret_cast<D3DKMT_HANDLE>(this);
		cc->hContext = context_;

		status = STATUS_SUCCESS;
	}

	return status;
}
