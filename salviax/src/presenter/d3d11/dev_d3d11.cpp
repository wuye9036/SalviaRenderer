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

#define INITGUID
#include "salviax/include/presenter/d3d11/dev_d3d11.h"
#include "salviar/include/framebuffer.h"
#include "salviar/include/surface.h"
#include <D3DX11.h>
#include <tchar.h>

#ifdef EFLIB_DEBUG
#	pragma comment(lib, "d3dx11d.lib")
#else
#	pragma comment(lib, "d3dx11.lib")
#endif

using namespace eflib;
using namespace salviar;

struct Vertex
{
	float x, y;
};

BEGIN_NS_SALVIAXPRESENTER()

HINSTANCE GetDLLHInstance()
{
	MEMORY_BASIC_INFORMATION mbi;
	if (VirtualQuery(GetDLLHInstance, &mbi, sizeof(mbi)))
	{
		return static_cast<HINSTANCE>(mbi.AllocationBase);
	}
	return NULL;
}

dev_d3d11::dev_d3d11(HWND hwnd): hwnd_(hwnd), gi_factory_(NULL), d3d_device_(NULL), d3d_imm_ctx_(NULL),
		swap_chain_(NULL), back_buffer_rtv_(NULL), buftex_(NULL), buftex_srv_(NULL), point_sampler_state_(NULL), vb_(NULL), input_layout_(NULL), vs_(NULL), ps_(NULL){
	// Dynamic loading because these dlls can't be loaded on WinXP
	mod_dxgi_ = ::LoadLibraryW(L"dxgi.dll");
	if (NULL == mod_dxgi_)
	{
		::MessageBoxW(NULL, L"Can't load dxgi.dll", L"Error", MB_OK);
	}
	mod_d3d11_ = ::LoadLibraryW(L"D3D11.dll");
	if (NULL == mod_d3d11_)
	{
		::MessageBoxW(NULL, L"Can't load d3d11.dll", L"Error", MB_OK);
	}

	if (mod_dxgi_ != NULL)
	{
		DynamicCreateDXGIFactory1_ = reinterpret_cast<CreateDXGIFactory1Func>(::GetProcAddress(mod_dxgi_, "CreateDXGIFactory1"));
	}

	if (mod_d3d11_ != NULL)
	{
		DynamicD3D11CreateDeviceAndSwapChain_ = reinterpret_cast<D3D11CreateDeviceAndSwapChainFunc>(::GetProcAddress(mod_d3d11_, "D3D11CreateDeviceAndSwapChain"));
	}

	DynamicCreateDXGIFactory1_(IID_IDXGIFactory1, reinterpret_cast<void**>(&gi_factory_));
}

dev_d3d11::~dev_d3d11(){
	d3d_imm_ctx_->ClearState();

	if (vs_){
		vs_->Release();
		vs_ = NULL;
	}
	if (ps_){
		ps_->Release();
		ps_ = NULL;
	}
	if (buftex_){
		buftex_->Release();
		buftex_ = NULL;
	}
	if (buftex_srv_){
		buftex_srv_->Release();
		buftex_srv_ = NULL;
	}
	if (point_sampler_state_){
		point_sampler_state_->Release();
		point_sampler_state_ = NULL;
	}
	if (vb_){
		vb_->Release();
		vb_ = NULL;
	}
	if (input_layout_){
		input_layout_->Release();
		input_layout_ = NULL;
	}

	if (d3d_imm_ctx_){
		d3d_imm_ctx_->Release();
		d3d_imm_ctx_ = NULL;
	}
	if (d3d_device_){
		d3d_device_->Release();
		d3d_device_ = NULL;
	}
	if (swap_chain_){
		swap_chain_->Release();
		swap_chain_ = NULL;
	}
	if (back_buffer_rtv_){
		back_buffer_rtv_->Release();
		back_buffer_rtv_ = NULL;
	}
	if (gi_factory_){
		gi_factory_->Release();
		gi_factory_ = NULL;
	}
}

h_dev_d3d11 dev_d3d11::create_device(HWND hwnd){
	return h_dev_d3d11(new dev_d3d11(hwnd));
}

//inherited
void dev_d3d11::present(const salviar::surface& surf)
{
	if (!d3d_device_)
	{
		DXGI_SWAP_CHAIN_DESC sc_desc;
		std::memset(&sc_desc, 0, sizeof(sc_desc));
		sc_desc.BufferCount = 1;
		sc_desc.BufferDesc.Width = static_cast<UINT>(surf.get_width());
		sc_desc.BufferDesc.Height = static_cast<UINT>(surf.get_height());
		sc_desc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		sc_desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		sc_desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		sc_desc.BufferDesc.RefreshRate.Numerator = 60;
		sc_desc.BufferDesc.RefreshRate.Denominator = 1;
		sc_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sc_desc.OutputWindow = hwnd_;
		sc_desc.SampleDesc.Count = 1;
		sc_desc.SampleDesc.Quality = 0;
		sc_desc.Windowed = true;
		sc_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		sc_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		UINT create_device_flags = 0;
#ifdef EFLIB_DEBUG
		create_device_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		D3D_FEATURE_LEVEL const feature_levels[] =
		{
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
			D3D_FEATURE_LEVEL_9_3,
			D3D_FEATURE_LEVEL_9_2,
			D3D_FEATURE_LEVEL_9_1
		};
		size_t const num_feature_levels = sizeof(feature_levels) / sizeof(feature_levels[0]);

		D3D_FEATURE_LEVEL out_feature_level;
		DynamicD3D11CreateDeviceAndSwapChain_(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, create_device_flags,
			feature_levels, num_feature_levels, D3D11_SDK_VERSION, &sc_desc, &swap_chain_, &d3d_device_,
			&out_feature_level, &d3d_imm_ctx_);

		ID3D11Texture2D* back_buffer;
		swap_chain_->GetBuffer(0, IID_ID3D11Texture2D, reinterpret_cast<void**>(&back_buffer));
		
		d3d_device_->CreateRenderTargetView(back_buffer, NULL, &back_buffer_rtv_);
		back_buffer->Release();

		d3d_imm_ctx_->OMSetRenderTargets(1, &back_buffer_rtv_, NULL);

		// Setup the viewport
		D3D11_VIEWPORT vp;
		vp.Width = static_cast<float>(surf.get_width());
		vp.Height = static_cast<float>(surf.get_height());
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		d3d_imm_ctx_->RSSetViewports(1, &vp);

		Vertex verts[] = 
		{
			/* x,  y */
			{-1.0f, +1.0f},
			{+1.0f, +1.0f},
			{-1.0f, -1.0f},
			{+1.0f, -1.0f}
		};

		D3D11_BUFFER_DESC buf_desc;
		buf_desc.ByteWidth = sizeof(verts);
		buf_desc.Usage = D3D11_USAGE_DEFAULT;
		buf_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		buf_desc.CPUAccessFlags = 0;
		buf_desc.MiscFlags = 0;
		buf_desc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA init_data;
		init_data.pSysMem = verts;
		init_data.SysMemPitch = sizeof(verts);
		init_data.SysMemSlicePitch = init_data.SysMemPitch;

		d3d_device_->CreateBuffer(&buf_desc, &init_data, &vb_);

		UINT strides[] = { sizeof(verts[0]) };
		UINT offsets[] = { 0 };
		d3d_imm_ctx_->IASetVertexBuffers(0, 1, &vb_, strides, offsets);

		HINSTANCE hModule = GetDLLHInstance();
		HRSRC rc;
		HGLOBAL global;
		LPCSTR rc_data;
		DWORD rc_size;

		rc = FindResource(hModule, TEXT("IDR_VS_HLSL"), TEXT("HLSL"));
		global = rc ? LoadResource(hModule, rc) : NULL;
		rc_data = global ? static_cast<LPCSTR>(LockResource(global)) : NULL;
		rc_size = rc ? SizeofResource(hModule, rc) : 0;
		ID3D10Blob* vs_code = NULL;
		ID3D10Blob* err_msg = NULL;
		D3DX11CompileFromMemory(rc_data, rc_size, NULL, NULL, NULL, "VSMain", "vs_4_0", 0, 0, NULL, &vs_code, &err_msg, NULL);
		if (err_msg)
		{
			std::cerr << err_msg->GetBufferPointer() << std::endl;
		}

		d3d_device_->CreateVertexShader(vs_code->GetBufferPointer(), vs_code->GetBufferSize(), NULL, &vs_);

		d3d_imm_ctx_->VSSetShader(vs_, NULL, 0);

		rc = FindResource(hModule, TEXT("IDR_PS_HLSL"), TEXT("HLSL"));
		global = rc ? LoadResource(hModule, rc) : NULL;
		rc_data = global ? static_cast<LPCSTR>(LockResource(global)) : NULL;
		rc_size = rc ? SizeofResource(hModule, rc) : 0;
		ID3D10Blob* ps_code = NULL;
		err_msg = NULL;
		D3DX11CompileFromMemory(rc_data, rc_size, NULL, NULL, NULL, "PSMain", "ps_4_0", 0, 0, NULL, &ps_code, &err_msg, NULL);
		if (err_msg)
		{
			std::cerr << err_msg->GetBufferPointer() << std::endl;
		}

		d3d_device_->CreatePixelShader(ps_code->GetBufferPointer(), ps_code->GetBufferSize(), NULL, &ps_);
		ps_code->Release();

		d3d_imm_ctx_->PSSetShader(ps_, NULL, 0);

		D3D11_INPUT_ELEMENT_DESC elems[1];
		elems[0].SemanticName = "POSITION";
		elems[0].SemanticIndex = 0;
		elems[0].Format = DXGI_FORMAT_R32G32_FLOAT;
		elems[0].InputSlot = 0;
		elems[0].AlignedByteOffset = 0;
		elems[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		elems[0].InstanceDataStepRate = 0;
		d3d_device_->CreateInputLayout(&elems[0], sizeof(elems) / sizeof(elems[0]), vs_code->GetBufferPointer(), vs_code->GetBufferSize(), &input_layout_);
		vs_code->Release();

		d3d_imm_ctx_->IASetInputLayout(input_layout_);

		D3D11_SAMPLER_DESC sampler_desc;
		sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampler_desc.MipLODBias = 0;
		sampler_desc.MaxAnisotropy = 0;
		sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampler_desc.BorderColor[0] = 0;
		sampler_desc.BorderColor[1] = 0;
		sampler_desc.BorderColor[2] = 0;
		sampler_desc.BorderColor[3] = 0;
		sampler_desc.MinLOD = 0;
		sampler_desc.MaxLOD = 0;
		d3d_device_->CreateSamplerState(&sampler_desc, &point_sampler_state_);

		d3d_imm_ctx_->PSSetSamplers(0, 1, &point_sampler_state_);

		d3d_imm_ctx_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		D3D11_TEXTURE2D_DESC tex_desc;
		tex_desc.Width = static_cast<UINT>(surf.get_width());
		tex_desc.Height = static_cast<UINT>(surf.get_height());
		tex_desc.MipLevels = 1;
		tex_desc.ArraySize = 1;
		tex_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		tex_desc.SampleDesc.Count = 1;
		tex_desc.SampleDesc.Quality = 0;
		tex_desc.Usage = D3D11_USAGE_DYNAMIC;
		tex_desc.BindFlags = D3D10_BIND_SHADER_RESOURCE;
		tex_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		tex_desc.MiscFlags = 0;
		d3d_device_->CreateTexture2D(&tex_desc, NULL, &buftex_);

		D3D11_SHADER_RESOURCE_VIEW_DESC sr_desc;
		sr_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		sr_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		sr_desc.Texture2D.MostDetailedMip = 0;
		sr_desc.Texture2D.MipLevels = 1;
		d3d_device_->CreateShaderResourceView(buftex_, &sr_desc, &buftex_srv_);

		d3d_imm_ctx_->PSSetShaderResources(0, 1, &buftex_srv_);
	}

	D3D11_MAPPED_SUBRESOURCE mapped;
	byte* src_addr = NULL;

	d3d_imm_ctx_->Map(buftex_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

	surf.map((void**)(&src_addr), map_read);
	if( src_addr == NULL ){
		d3d_imm_ctx_->Unmap(buftex_, 0);
		return;
	}

	for(size_t irow = 0; irow < surf.get_height(); ++irow)
	{
		byte* dest_addr = ((byte*)(mapped.pData)) + mapped.RowPitch * irow;
		pixel_format_convertor::convert_array(
			pixel_format_color_bgra8, surf.get_pixel_format(),
			dest_addr, src_addr,
			static_cast<int>(surf.get_width())
			);
		src_addr += surf.get_pitch();
	}

	surf.unmap();
	d3d_imm_ctx_->Unmap(buftex_, 0);

	d3d_imm_ctx_->Draw(4, 0);

	swap_chain_->Present(0, 0);
}

END_NS_SALVIAXPRESENTER()

void salviax_create_presenter_device(salviar::h_device& dev, void* param)
{
	dev = softartx::presenter::dev_d3d11::create_device(static_cast<HWND>(param));
}

