#ifndef SALVIAR_SHADER_IMPL_H
#define SALVIAR_SHADER_IMPL_H

#include <salviar/include/salviar_forward.h>

#include <string>

BEGIN_NS_SALVIAR();

struct external_function_desc
{
	external_function_desc(void* func, std::string const& func_name, bool is_raw_name)
		:func(func), func_name(func_name), is_raw_name(is_raw_name)
	{
	}
	void*		func;
	std::string func_name;
	bool		is_raw_name;
};

END_NS_SALVIAR();

#endif