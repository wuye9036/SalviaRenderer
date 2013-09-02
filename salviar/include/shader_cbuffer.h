
#pragma once

#include <salviar/include/salviar_forward.h>

#include <salviar/include/shader_cbuffer.h>
#include <eflib/include/utility/shared_declaration.h>
#include <eflib/include/string/ustring.h>

#include <unordered_map>
#include <type_traits>

BEGIN_NS_SALVIAR();

EFLIB_DECLARE_CLASS_SHARED_PTR(texture);
EFLIB_DECLARE_CLASS_SHARED_PTR(sampler);

enum class shader_cdata_type: uint32_t
{
	sdt_none,
	sdt_pod,
	sdt_sampler
};

struct shader_cdata
{
	shader_cdata()
		: offset(0), length(0), array_size(0)
	{
	}

	size_t				offset;
	size_t				length;
	size_t				array_size;
};

class shader_cbuffer
{
public:
	typedef std::unordered_map<
		eflib::fixed_string, shader_cdata
	> variable_table;
	typedef std::unordered_map<
		eflib::fixed_string, sampler_ptr
	> sampler_table;

	virtual void set_sampler(eflib::fixed_string const& name, sampler_ptr const& samp);
	virtual void set_variable(eflib::fixed_string const& name, void const* data, size_t data_length);
	
	variable_table const&	variables() const
	{
		return variables_;
	}

	sampler_table  const&	samplers()  const
	{
		return samplers_;
	}

	void const* data_pointer(shader_cdata const& cdata) const
	{
		if(cdata.length == 0)
		{
			return nullptr;
		}
		return data_memory_.data() + cdata.offset;
	}
	
    void copy_from(shader_cbuffer const* src)
    {
        *this = *src;
    }
private:
	variable_table				variables_;
	std::vector<char>			data_memory_;
	sampler_table				samplers_;
	std::vector<texture_ptr>	textures_;
};

END_NS_SALVIAR();
