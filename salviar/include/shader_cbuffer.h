#pragma once

#include <salviar/include/salviar_forward.h>

#include <eflib/include/utility/shared_declaration.h>
#include <eflib/include/string/ustring.h>

#include <type_traits>

BEGIN_NS_SALVIAR();

EFLIB_DECLARE_CLASS_SHARED_PTR(sampler);

class shader_cbuffer
{
public:
	template <typename T>
	void set_variable(eflib::fixed_string const& name, T const& v)
	{
		static_assert(!std::is_pointer<T>::value);
		set_variable( name, &v, sizeof(v) );
	}
	
	virtual void set_sampler(eflib::fixed_string const& name, sampler_ptr const& samp) = 0;
	
protected:	
	virtual void set_variable(eflib::fixed_string const& name, void const* data, size_t data_length) = 0;
};

END_NS_SALVIAR();
