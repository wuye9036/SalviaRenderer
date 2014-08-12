#define INITGUID
#include <salvia_d3d_sw_driver/include/common.h>

#include <eflib/include/platform/typedefs.h>

#include <salvia_d3d_sw_driver/include/umd_device.h>
#include <salvia_d3d_sw_driver/include/umd_resource.h>

#include <salviar/include/renderer.h>
#include <salviar/include/buffer.h>
#include <salviar/include/texture.h>
#include <salviar/include/internal_mapped_resource.h>

using namespace salviar;

umd_resource::umd_resource(umd_device* dev, const D3D11DDIARG_CREATERESOURCE* create_resource,
		D3D10DDI_HRTRESOURCE rt_resource)
	: creation_param_(*create_resource)
{
	UNREFERENCED_PARAMETER(rt_resource);

	const uint32_t surfs = create_resource->ArraySize * create_resource->MipLevels;
	
	mip_info_list_.assign(create_resource->pMipInfoList, create_resource->pMipInfoList + create_resource->MipLevels);
	creation_param_.pMipInfoList = &mip_info_list_[0];
	if (create_resource->pInitialDataUP != NULL)
	{
		initial_data_up_.assign(create_resource->pInitialDataUP, create_resource->pInitialDataUP + surfs);
		creation_param_.pInitialDataUP = &initial_data_up_[0];
	}

	// TODO: support other types of resources
	if (D3D10DDIRESOURCE_BUFFER == create_resource->ResourceDimension)
	{
		buf_ = dev->sa_renderer_->create_buffer(create_resource->pMipInfoList[0].TexelWidth);
		if (creation_param_.pInitialDataUP)
		{
			buf_->transfer(0, creation_param_.pInitialDataUP[0].pSysMem, 1, create_resource->pMipInfoList[0].TexelWidth);
		}
	}
	else
	{
		pixel_format fmt;
		switch (create_resource->Format)
		{
		case DXGI_FORMAT_R32G32B32A32_FLOAT:
			fmt = pixel_format_color_rgba32f;
			break;
		case DXGI_FORMAT_R32G32B32_FLOAT:
			fmt = pixel_format_color_rgb32f;
			break;
		case DXGI_FORMAT_B8G8R8A8_UNORM:
			fmt = pixel_format_color_bgra8;
			break;
		case DXGI_FORMAT_R8G8B8A8_UNORM:
			fmt = pixel_format_color_rgba8;
			break;
		case DXGI_FORMAT_R32_FLOAT:
			fmt = pixel_format_color_r32f;
			break;
		case DXGI_FORMAT_R32G32_FLOAT:
			fmt = pixel_format_color_rg32f;
			break;
		case DXGI_FORMAT_R32_SINT:
			fmt = pixel_format_color_r32i;
			break;

		default:
			assert(false);
			fmt = pixel_format_color_rgba8;
			break;
		}

		switch (create_resource->ResourceDimension)
		{
		case D3D10DDIRESOURCE_TEXTURE2D:
			tex2d_ = dev->sa_renderer_->create_tex2d(create_resource->pMipInfoList[0].TexelWidth,
				create_resource->pMipInfoList[0].TexelHeight,
				create_resource->SampleDesc.Count,
				fmt);
			// TODO: Fill in data
			/*if (creation_param_.pInitialDataUP)
			{
				for (uint32_t i = 0; i < surfs; ++i)
				{
					eflib::rect<int> rc(0, 0, create_resource->pMipInfoList[0].TexelWidth, create_resource->pMipInfoList[0].TexelHeight);
					tex2d_->subresource(i)->transfer(fmt, rc, creation_param_.pInitialDataUP[i].pSysMem);
				}
			}*/
			break;

		default:
			assert(false);
			break;
		}
	}
}

umd_resource::~umd_resource()
{
	this->destroy();
}

void umd_resource::destroy()
{
	// TODO: support other types of resources
	switch (creation_param_.ResourceDimension)
	{
	case D3D10DDIRESOURCE_BUFFER:
		buf_.reset();
		break;

	case D3D10DDIRESOURCE_TEXTURE2D:
		tex2d_.reset();
		break;

	default:
		assert(false);
		break;
	}
}
