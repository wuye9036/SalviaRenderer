/*
Copyright (C) 2007-2010 Minmin Gong, Ye Wu

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

#include <salviax/include/presenter/d3d9/dev_d3d9.h>
#include <salviar/include/framebuffer.h>
#include <salviar/include/surface.h>
#include <tchar.h>

using namespace eflib;
using namespace salviax::utility;
using namespace salviar;
#define FVF (D3DFVF_XYZ | D3DFVF_TEX1)
struct Vertex
{
	float x, y, z;
	float s, t;
};

BEGIN_NS_SALVIAX_PRESENTER();

dev_d3d9::dev_d3d9(HWND hwnd, h_d3d9_device dev): hwnd_(hwnd), dev_(dev), buftex_(NULL), vb_(NULL){
	init_device();
}

dev_d3d9::~dev_d3d9(){
	if (buftex_){
		buftex_->Release();
		buftex_ = NULL;
	}
	if (vb_){
		vb_->Release();
		vb_ = NULL;
	}
}

void dev_d3d9::init_device()
{
	if (dev_)
	{
		IDirect3DDevice9* pdxdev = dev_->get_d3d_device9();

		pdxdev->CreateVertexBuffer(
			4 * sizeof(Vertex), D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &vb_, NULL);
		
		pdxdev->SetStreamSource(0, vb_, 0, sizeof(Vertex));
		pdxdev->SetFVF(FVF);
		pdxdev->SetRenderState(D3DRS_LIGHTING, FALSE);
		pdxdev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
		pdxdev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		pdxdev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	}
}

h_dev_d3d9 dev_d3d9::create_device(HWND hwnd, h_d3d9_device dev){
	return h_dev_d3d9(new dev_d3d9( hwnd, dev ));
}

//inherited
void dev_d3d9::present(const salviar::surface& surf)
{
	if (!dev_)
	{
		D3DPRESENT_PARAMETERS d3dpp;
		std::memset(&d3dpp, 0, sizeof(d3dpp));
		d3dpp.Windowed					= true;
		d3dpp.BackBufferCount			= 1;
		d3dpp.BackBufferWidth			= static_cast<UINT>(surf.get_width());
		d3dpp.BackBufferHeight			= static_cast<UINT>(surf.get_height());
		d3dpp.hDeviceWindow				= hwnd_;
		d3dpp.SwapEffect				= D3DSWAPEFFECT_DISCARD;
		d3dpp.PresentationInterval		= D3DPRESENT_INTERVAL_IMMEDIATE;
		d3dpp.BackBufferFormat			= D3DFMT_X8R8G8B8;

		salviax::utility::d3d9_device_param device_params;
		device_params.adapter = 0;
		device_params.devtype = D3DDEVTYPE_HAL;
		device_params.focuswnd = d3dpp.hDeviceWindow;
		device_params.behavior = D3DCREATE_HARDWARE_VERTEXPROCESSING;

		dev_ = salviax::utility::d3d9_device::create(device_params, d3dpp);

		init_device();
	
		IDirect3DDevice9* pdxdev = dev_->get_d3d_device9();

		Vertex verts[] = 
		{
			/* x,  y, z, u, v */
			{-1.0f - 1.0f / surf.get_width(), +1.0f + 1.0f / surf.get_height(), 0.5f, 0.0f, 0.0f},
			{ 1.0f - 1.0f / surf.get_width(), +1.0f + 1.0f / surf.get_height(), 0.5f, 1.0f, 0.0f},
			{-1.0f - 1.0f / surf.get_width(), -1.0f + 1.0f / surf.get_height(), 0.5f, 0.0f, 1.0f},
			{ 1.0f - 1.0f / surf.get_width(), -1.0f + 1.0f / surf.get_height(), 0.5f, 1.0f, 1.0f}
		};
		Vertex* p;
		vb_->Lock(0, 0, reinterpret_cast<void**>(&p), 0);
		memcpy(p, verts, sizeof(verts));
		vb_->Unlock();

		HRESULT hr = pdxdev->CreateTexture(
			static_cast<uint32_t>(surf.get_width()), static_cast<uint32_t>(surf.get_height()),
			1, D3DUSAGE_DYNAMIC, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &buftex_, NULL);

		if(FAILED(hr)){
			buftex_ = NULL;
		}

		pdxdev->SetTexture(0, buftex_);
	}

	// Copy frame buffer to texture.
	D3DLOCKED_RECT locked_rc;
	byte* src_addr = NULL;

	if( FAILED(buftex_->LockRect(0, &locked_rc, NULL, D3DLOCK_DISCARD)) ){
		return;
	}

	surf.map((void**)(&src_addr), map_read);
	if( src_addr == NULL ) return;

	for(size_t irow = 0; irow < surf.get_height(); ++irow)
	{
		byte* dest_addr = ((byte*)(locked_rc.pBits)) + locked_rc.Pitch * irow;
		pixel_format_convertor::convert_array(
			pixel_format_color_bgra8, surf.get_pixel_format(),
			dest_addr, src_addr,
			int(surf.get_width())
			);
		src_addr += surf.get_pitch();
	}

	surf.unmap();
	buftex_->UnlockRect(0);

	IDirect3DDevice9* pdxdev = dev_->get_d3d_device9();

	pdxdev->BeginScene();
	pdxdev->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	pdxdev->EndScene();

	RECT rc;
	rc.left = 0;
	rc.top = 0;
	rc.right = static_cast<LONG>(surf.get_width());
	rc.bottom = static_cast<LONG>(surf.get_height());
	pdxdev->Present(NULL, &rc, NULL, NULL);
}

END_NS_SALVIAX_PRESENTER()

void salviax_create_presenter_device(salviar::h_device& dev, void* param)
{
	dev = salviax::presenter::dev_d3d9::create_device(static_cast<HWND>(param));
}

