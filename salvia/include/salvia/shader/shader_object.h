#ifndef SALVIAR_SHADER_OBJECT_H
#define SALVIAR_SHADER_OBJECT_H

#include <salviar/include/salviar_forward.h>

#include <salviar/include/shader.h>
#include <salvia/shader/reflection.h>

#include <eflib/utility/shared_declaration.h>

namespace salviar{

class shader_reflection;

EFLIB_DECLARE_CLASS_SHARED_PTR(shader_log);
class shader_log
{
public:
	virtual size_t				count() const = 0;
	virtual std::string const&	log_string(size_t index) const = 0;
};

EFLIB_DECLARE_CLASS_SHARED_PTR(shader_object);
class shader_object{
public:
	virtual shader_reflection const* get_reflection() const = 0;
	virtual void* 					 native_function() const = 0;

	template <typename FuncPtrT> FuncPtrT native_function() const
	{
		return reinterpret_cast<FuncPtrT>( native_function() );
	}
};

}

#endif