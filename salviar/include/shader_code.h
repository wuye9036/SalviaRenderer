#ifndef SALVIAR_SHADER_CODE_H
#define SALVIAR_SHADER_CODE_H

#include <salviar/include/salviar_forward.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

BEGIN_NS_SALVIAR();

class shader_code_impl;

class shader_code{
public:
	virtual abi_info const* abii() const;
	virtual void abii( abi_info const* );
	
	virtual void* function_pointer() const;

	static boost::shared_ptr<shader_code> create();
};

END_NS_SALVIAR();

#endif