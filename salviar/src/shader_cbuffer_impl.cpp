#include <salviar/include/shader_cbuffer_impl.h>
using eflib::fixed_string;

BEGIN_NS_SALVIAR();

void shader_cbuffer_impl::set_sampler(eflib::fixed_string const& name, sampler_ptr const& samp)
{
	auto iter = samplers_.find(name);
	if (iter != samplers_.end() )
	{
		iter->second = samp;
	}
	else
	{
		samplers_.emplace(name, samp);
	}
}

void shader_cbuffer_impl::set_variable(eflib::fixed_string const& name, void const* data, size_t data_length)
{
	shader_cdata* existed_cdata = nullptr;

	auto existed_variable_iter = variables_.find(name);
	if(existed_variable_iter != variables_.end())
	{
		existed_cdata = &(existed_variable_iter->second);
	}

	size_t	offset = 0;
	if(existed_cdata != nullptr && existed_cdata->length >= data_length)
	{
		offset = existed_cdata->offset;
		existed_cdata->length = data_length;
	}
	else
	{
		offset = data_memory_.size();

		shader_cdata cdata;
		cdata.array_size= 0;
		cdata.length	= data_length;
		cdata.offset	= offset;
		data_memory_.resize(data_memory_.size() + data_length);
	
		if(existed_cdata == nullptr)
		{
			variables_.emplace(name, cdata);
		}
		else
		{
			*existed_cdata = cdata;
		}
	}

	memcpy(data_memory_.data()+offset, data, data_length);
}

END_NS_SALVIAR();
