#define INITGUID
#include <salvia_d3d_sw_driver/include/common.h>

#include <eflib/include/platform/typedefs.h>

#include <salvia_d3d_sw_driver/include/adapter.h>
#include <salvia_d3d_sw_driver/include/device.h>

umd_device::umd_device(umd_adapter* adapter, const D3D10DDIARG_CREATEDEVICE* args)
    : adapter_(adapter)
{
    UNREFERENCED_PARAMETER(args);
}
