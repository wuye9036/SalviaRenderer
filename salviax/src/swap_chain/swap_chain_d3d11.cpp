#if defined(SALVIAX_D3D11_ENABLED)

#include <salviax/include/swap_chain/swap_chain_impl.h>

#include <salviar/include/surface.h>
#include <salviar/include/renderer.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/make_shared.hpp>
#include <eflib/include/platform/boost_end.h>

#ifndef NOMINMAX
#	define NOMINMAX
#endif

#include <DXGI.h>
#include <D3D11.h>
#include <D3DX11.h>

#include <tchar.h>

#ifdef EFLIB_DEBUG
#	pragma comment(lib, "d3dx11d.lib")
#	pragma comment(lib, "dxguid.lib")
#else
#	pragma comment(lib, "d3dx11.lib")
#	pragma comment(lib, "dxguid.lib")
#endif

using namespace eflib;
using namespace salviar;

struct quad_vertex
{
	float x, y;
};

BEGIN_NS_SALVIAX();

HINSTANCE get_dll_instance()
{
	MEMORY_BASIC_INFORMATION mbi;
	if (VirtualQuery(get_dll_instance, &mbi, sizeof(mbi)))
	{
		return static_cast<HINSTANCE>(mbi.AllocationBase);
	}
	return NULL;
}

char const ps_data[] =
    "Texture2D sa_tex : register(t0); \n"
    "SamplerState point_sampler : register(s0);\n"
    "float4 PSMain(float2 tex0 : TEXCOORD0) : SV_Target\n"
    "{\n"
	"   return sa_tex.Sample(point_sampler, tex0);\n"
    "}\n"
    ;

char const vs_data[] =
    "void VSMain(float2 pos : POSITION,\n"
	"	out float2 oTex0 : TEXCOORD0,\n"
	"	out float4 oPos : SV_Position)\n"
    "{\n"
	"   oPos = float4(pos, 0, 1);\n"
	"   oTex0 = float2(float2(pos.x, -pos.y) * 0.5f + 0.5f);\n"
    "}\n"
    ;

class d3d11_swap_chain: public swap_chain_impl
{
public:
	d3d11_swap_chain(
		renderer_ptr const& renderer,
		renderer_parameters const& params )
		: swap_chain_impl(renderer, params)
		, hwnd_(nullptr)
		, mod_dxgi_(nullptr), mod_d3d11_(nullptr)
		, gi_factory_(nullptr)
		, d3d_device_(nullptr), d3d_imm_ctx_(nullptr)
		, swap_chain_(nullptr)
		, back_buffer_rtv_(nullptr)
		, buftex_(nullptr), buftex_srv_(nullptr)
		, point_sampler_state_(nullptr)
		, vb_(nullptr), input_layout_(nullptr)
		, vs_(nullptr), ps_(nullptr)
	{
		hwnd_ = reinterpret_cast<HWND>(params.native_window);
		initialize();
	}

	~d3d11_swap_chain()
	{
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

	void initialize()
	{
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
			DynamicCreateDXGIFactory1_ =
				reinterpret_cast<CreateDXGIFactory1Func>(
					::GetProcAddress(mod_dxgi_, "CreateDXGIFactory1")
				);
		}

		if (mod_d3d11_ != NULL)
		{
			DynamicD3D11CreateDeviceAndSwapChain_ =
				reinterpret_cast<D3D11CreateDeviceAndSwapChainFunc>(
					::GetProcAddress(mod_d3d11_, "D3D11CreateDeviceAndSwapChain")
				);
		}

		DynamicCreateDXGIFactory1_(IID_IDXGIFactory1, reinterpret_cast<void**>(&gi_factory_));
	}

	void present_impl()
	{
		if (!d3d_device_)
		{
			DXGI_SWAP_CHAIN_DESC sc_desc;
			std::memset(&sc_desc, 0, sizeof(sc_desc));
			sc_desc.BufferCount = 1;
			sc_desc.BufferDesc.Width = static_cast<UINT>(resolved_surface_->get_width());
			sc_desc.BufferDesc.Height = static_cast<UINT>(resolved_surface_->get_height());
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
			vp.Width = static_cast<float>(resolved_surface_->get_width());
			vp.Height = static_cast<float>(resolved_surface_->get_height());
			vp.MinDepth = 0.0f;
			vp.MaxDepth = 1.0f;
			vp.TopLeftX = 0;
			vp.TopLeftY = 0;
			d3d_imm_ctx_->RSSetViewports(1, &vp);

			quad_vertex verts[] = 
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

			ID3D10Blob* vs_code = NULL;
			ID3D10Blob* err_msg = NULL;
			D3DX11CompileFromMemory(vs_data, sizeof(vs_data), NULL, NULL, NULL, "VSMain", "vs_4_0", 0, 0, NULL, &vs_code, &err_msg, NULL);
			if (err_msg)
			{
				std::cerr << err_msg->GetBufferPointer() << std::endl;
			}

			d3d_device_->CreateVertexShader(vs_code->GetBufferPointer(), vs_code->GetBufferSize(), NULL, &vs_);

			d3d_imm_ctx_->VSSetShader(vs_, NULL, 0);

			ID3D10Blob* ps_code = NULL;
			err_msg = NULL;
			D3DX11CompileFromMemory(ps_data, sizeof(ps_data), NULL, NULL, NULL, "PSMain", "ps_4_0", 0, 0, NULL, &ps_code, &err_msg, NULL);
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
			tex_desc.Width = static_cast<UINT>(resolved_surface_->get_width());
			tex_desc.Height = static_cast<UINT>(resolved_surface_->get_height());
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

		resolved_surface_->map((void**)(&src_addr), map_read);
		if( src_addr == NULL ){
			d3d_imm_ctx_->Unmap(buftex_, 0);
			return;
		}

		for(size_t irow = 0; irow < resolved_surface_->get_height(); ++irow)
		{
			byte* dest_addr = ((byte*)(mapped.pData)) + mapped.RowPitch * irow;
			pixel_format_convertor::convert_array(
				pixel_format_color_bgra8, resolved_surface_->get_pixel_format(),
				dest_addr, src_addr,
				static_cast<int>(resolved_surface_->get_width())
				);
			src_addr += resolved_surface_->get_pitch();
		}

		resolved_surface_->unmap();
		d3d_imm_ctx_->Unmap(buftex_, 0);

		d3d_imm_ctx_->Draw(4, 0);

		swap_chain_->Present(0, 0);
	}

private:
	typedef HRESULT (WINAPI *CreateDXGIFactory1Func)(REFIID riid, void** ppFactory);
	typedef HRESULT (WINAPI *D3D11CreateDeviceAndSwapChainFunc)(IDXGIAdapter* pAdapter,
		D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags,
		D3D_FEATURE_LEVEL const * pFeatureLevels, UINT FeatureLevels,
		UINT SDKVersion,
		DXGI_SWAP_CHAIN_DESC* pSwapChainDesc, IDXGISwapChain** ppSwapChain,
		ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel,
		ID3D11DeviceContext** ppImmediateContext);

	CreateDXGIFactory1Func				DynamicCreateDXGIFactory1_;
	D3D11CreateDeviceAndSwapChainFunc	DynamicD3D11CreateDeviceAndSwapChain_;

	HWND						hwnd_;
	HMODULE						mod_dxgi_;
	HMODULE						mod_d3d11_;

	IDXGIFactory*				gi_factory_;
	ID3D11Device*				d3d_device_;
	ID3D11DeviceContext*		d3d_imm_ctx_;
	IDXGISwapChain*				swap_chain_;
	ID3D11RenderTargetView*		back_buffer_rtv_;
	ID3D11Texture2D*			buftex_;
	ID3D11ShaderResourceView*	buftex_srv_;
	ID3D11SamplerState*			point_sampler_state_;
	ID3D11Buffer*				vb_;
	ID3D11InputLayout*			input_layout_;
	ID3D11VertexShader*			vs_;
	ID3D11PixelShader*			ps_;
};

swap_chain_ptr create_d3d11_swap_chain(
	renderer_ptr const& renderer,
	renderer_parameters const* params)
{
	if(!params)
	{
		return swap_chain_ptr();
	}

	return boost::make_shared<d3d11_swap_chain>(renderer, *params);
}

END_NS_SALVIAX();

#endif
