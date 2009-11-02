/*
Copyright (C) 2004-2005 Minmin Gong

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published
by the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "softartx/include/user_config.h"
#include "softartx/include/presenter/dev_d3d9.h"

#include "softart/include/framebuffer.h"
#include "softart/include/surface.h"

#include <tchar.h>

using namespace efl;

#define fvf (D3DFVF_XYZ | D3DFVF_TEX1)
struct Vertex
{
	float x, y, z;
	float s, t;
};

IDirect3D9* d3d9_device::pd3d9_(NULL);

d3d9_device::d3d9_device()
{
	if(pd3d9_ == NULL){
		pd3d9_ = Direct3DCreate9(D3D_SDK_VERSION);
	} else {
		pd3d9_->AddRef();
	}

	devinfo_.pdevice = NULL;
	devinfo_.dev_type = devtype_d3d9;

	ptex_ = NULL;
}

d3d9_device::~d3d9_device()
{
	if(ptex_)
		ptex_->Release();
	if(devinfo_.pdevice)
		((IDirect3DDevice9*)(devinfo_.pdevice))->Release();	
	if(pd3d9_)
		pd3d9_->Release();
}

h_d3d9_device d3d9_device::create_device(
	const d3d9_dev_param* dev_param,
	D3DPRESENT_PARAMETERS* present_param
	)
{
	h_d3d9_device hdev(new d3d9_device());

	IDirect3DDevice9* pdxdev = NULL;
	HRESULT hr = pd3d9_->CreateDevice(
		dev_param->adapter,
		dev_param->devtype,
		dev_param->focuswnd,
		dev_param->behavior,
		present_param, &pdxdev
		);
	if(FAILED(hr)) return h_d3d9_device();

	hdev->devinfo_.dev_type = devtype_d3d9;
	hdev->devinfo_.pdevice = (intptr_t)pdxdev;

	return hdev;
}

//inherited
void d3d9_device::attach_framebuffer(framebuffer* pfb)
{
	if(ptex_){
		ptex_->Release();
	}

	if(pfb == NULL) return;

	pfb_ = pfb;
	HRESULT hr = ((IDirect3DDevice9*)(devinfo_.pdevice))->CreateTexture(
		uint32_t(pfb->get_width()), uint32_t(pfb->get_height()),
		1, D3DUSAGE_DYNAMIC, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &ptex_, NULL);

	if(FAILED(hr))
	{
		ptex_ = NULL; 
		pfb_ = NULL;
		return;
	}
}

const device_info& d3d9_device::get_physical_device()
{
	return devinfo_;
}

void d3d9_device::present(const rect<size_t>& /*src_rect*/, const rect<size_t>& /*dest_rect*/)
{
	HRESULT hr;

	if(!ptex_) return;

	//首先将framebuffer copy到纹理上
	D3DLOCKED_RECT locked_rc;
	byte* psrc = NULL;
	surface* prt = pfb_->get_render_target(render_target_color, 0);
	hr = ptex_->LockRect(0, &locked_rc, NULL, 0);
	prt->lock((void**)(&psrc), pfb_->get_rect(), lock_read_only);

	if( psrc == NULL || FAILED(hr) ) return;

	for(size_t irow = 0; irow < pfb_->get_height(); ++irow)
	{
		pixel_format_convertor::convert_array(
			pixel_format_color_bgra8, prt->get_pixel_format(),
			((byte*)(locked_rc.pBits)) + locked_rc.Pitch * irow, psrc,
			int(prt->get_width())
			);
		psrc += prt->get_pitch();
	}

	prt->unlock();
	ptex_->UnlockRect(0);

	//设置渲染顶点
	Vertex verts[] = 
	{
		/*
		     x,      y,    z,     u,     v
		*/
		{-1.0f, -1.0f, 0.5f, 0.0f, 0.0f},
		{1.0f, -1.0f, 0.5f, 1.0f, 0.0f},
		{-1.0f, 1.0f, 0.5f, 0.0f, 1.0f},
		{1.0f, 1.0f, 0.5f, 1.0f, 1.0f}
	};

	IDirect3DDevice9* pdxdev = (IDirect3DDevice9*)(devinfo_.pdevice);

	pdxdev->SetFVF(fvf);
	pdxdev->SetTexture(0, ptex_);
	
	pdxdev->SetRenderState(D3DRS_LIGHTING, FALSE);
	pdxdev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	pdxdev->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);

	pdxdev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	pdxdev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);

	hr = pdxdev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(64, 64, 128), 1.0f, 0);

	pdxdev->BeginScene();
	pdxdev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, &verts[0], sizeof(Vertex));
	pdxdev->EndScene();

	hr = pdxdev->Present(NULL, NULL, NULL, NULL);
}