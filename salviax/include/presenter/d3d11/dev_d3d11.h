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

#ifndef SALVIAX_DEV_D3D11_H
#define SALVIAX_DEV_D3D11_H

#ifndef NOMINMAX
#	define NOMINMAX
#endif

#include <DXGI.h>
#include <D3D11.h>
#include "salviar/include/presenter_dev.h"
#include <eflib/include/math/math.h>
#include <boost/smart_ptr.hpp>

#define BEGIN_NS_SALVIAX_PRESENTER() namespace softartx{ namespace presenter{
#define END_NS_SALVIAX_PRESENTER() }}

BEGIN_NS_SALVIAX_PRESENTER()

class dev_d3d11;
DECL_HANDLE(dev_d3d11, h_dev_d3d11);

class dev_d3d11 : public salviar::device
{
public:
	static h_dev_d3d11 create_device(HWND hwnd);

	//inherited
	virtual void present(const salviar::surface& surf);

	~dev_d3d11();

private:
	dev_d3d11(HWND hwnd);

	HWND hwnd_;

	HMODULE mod_dxgi_;
	HMODULE mod_d3d11_;

	typedef HRESULT (WINAPI *CreateDXGIFactory1Func)(REFIID riid, void** ppFactory);
	typedef HRESULT (WINAPI *D3D11CreateDeviceAndSwapChainFunc)(IDXGIAdapter* pAdapter,
							D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags,
							D3D_FEATURE_LEVEL const * pFeatureLevels, UINT FeatureLevels, UINT SDKVersion,
							DXGI_SWAP_CHAIN_DESC* pSwapChainDesc, IDXGISwapChain** ppSwapChain,
							ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext);

	CreateDXGIFactory1Func DynamicCreateDXGIFactory1_;
	D3D11CreateDeviceAndSwapChainFunc DynamicD3D11CreateDeviceAndSwapChain_;

	IDXGIFactory* gi_factory_;
	ID3D11Device* d3d_device_;
	ID3D11DeviceContext* d3d_imm_ctx_;
	IDXGISwapChain* swap_chain_;
	ID3D11RenderTargetView* back_buffer_rtv_;
	ID3D11Texture2D* buftex_;
	ID3D11ShaderResourceView* buftex_srv_;
	ID3D11SamplerState* point_sampler_state_;
	ID3D11Buffer* vb_;
	ID3D11InputLayout* input_layout_;
	ID3D11VertexShader* vs_;
	ID3D11PixelShader* ps_;
};

END_NS_SALVIAX_PRESENTER()

#ifdef salviax_d3d11_presenter_EXPORTS
	#define SALVIAX_API __declspec(dllexport)
#else
	#define SALVIAX_API __declspec(dllimport)
#endif

extern "C"
{
	SALVIAX_API void salviax_create_presenter_device(salviar::h_device& dev, void* param);
}

#endif //SALVIAX_DEV_D3D11_H
