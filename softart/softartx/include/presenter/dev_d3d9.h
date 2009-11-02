#ifndef SOFTARTX_DEV_D3D9_H
#define SOFTARTX_DEV_D3D9_H

#include "dev.h"

#include "eflib/include/math.h"
#include "eflib/include/platform.h"

#include "softartx/include/inc_d3d9.h"

#include <boost/smart_ptr.hpp>

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

struct d3d9_dev_param
{
	uint32_t adapter;
	D3DDEVTYPE devtype;
	HWND focuswnd;
	DWORD behavior;
};

class d3d9_device;
DECL_HANDLE(d3d9_device, h_d3d9_device);

class d3d9_device : public device_impl
{
	static IDirect3D9* pd3d9_;

	device_info devinfo_;

	IDirect3DTexture9* ptex_;
	framebuffer* pfb_;

	d3d9_device();

public:

	~d3d9_device();
	
	static h_d3d9_device create_device(
		const d3d9_dev_param* dev_param,
		D3DPRESENT_PARAMETERS* present_param
		);

	//inherited
	virtual void attach_framebuffer(framebuffer* pfb);
	virtual const device_info& get_physical_device();
	virtual void present(const efl::rect<size_t>& src_rect, const efl::rect<size_t>& dest_rect);
};

#endif //SOFTARTX_DEV_D3D9_H
