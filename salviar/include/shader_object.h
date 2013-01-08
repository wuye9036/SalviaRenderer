#ifndef SALVIAR_SHADER_OBJECT_H
#define SALVIAR_SHADER_OBJECT_H

#include <salviar/include/salviar_forward.h>

#include <salviar/include/shader.h>
#include <salviar/include/shader_reflection.h>

#include <eflib/include/utility/shared_declaration.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

BEGIN_NS_SALVIAR();

class shader_reflection;

EFLIB_DECLARE_CLASS_SHARED_PTR(shader_log);
class shader_log
{
	virtual size_t				count() const = 0;
	virtual std::string const&	log_string(size_t index) const = 0;
};

EFLIB_DECLARE_CLASS_SHARED_PTR(shader_object);
class shader_object{
public:
	virtual shader_reflection const* get_reflection() const = 0;
	virtual void* 					 native_function() const = 0;
};

END_NS_SALVIAR();

#endif