#pragma once

#include <vector>

class umd_resource
{
public:
	umd_resource(umd_device* dev, const D3D11DDIARG_CREATERESOURCE* create_resource,
		D3D10DDI_HRTRESOURCE rt_resource);
	~umd_resource();

	void destroy();

	const salvia::resource::buffer_ptr& buffer() const
	{
		return buf_;
	}
	const salvia::resource::texture_ptr& texture_2d() const
	{
		return tex2d_;
	}

private:
	D3D11DDIARG_CREATERESOURCE creation_param_;
	std::vector<D3D10DDI_MIPINFO> mip_info_list_;
	std::vector<D3D10_DDIARG_SUBRESOURCE_UP> initial_data_up_;

	salvia::resource::buffer_ptr buf_;
	salvia::resource::texture_ptr tex2d_;
};
