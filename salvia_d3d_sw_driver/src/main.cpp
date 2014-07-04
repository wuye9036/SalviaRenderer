#include <windows.h>

#include <salvia_d3d_sw_driver/include/display.h>

extern void km_destroy();

HINSTANCE g_dll;

BOOL WINAPI DllMain(HINSTANCE hmod, UINT reason, LPVOID reserved)
{
	UNREFERENCED_PARAMETER(reserved);

	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
		g_dll = hmod;
		break;

	case DLL_PROCESS_DETACH:
		display_mgr::destroy();
		km_destroy();
		break;
	}

	return TRUE;
}
