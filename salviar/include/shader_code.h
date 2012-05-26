#ifndef SALVIAR_SHADER_CODE_H
#define SALVIAR_SHADER_CODE_H

#include <salviar/include/salviar_forward.h>

#include <salviar/include/shader.h>
#include <salviar/include/shader_abi.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

namespace sasl{
	namespace semantic{
		class abi_info;
		class module_si;
	}
	namespace code_generator{
		class jit_engine;
	}
}

BEGIN_NS_SALVIAR();

class shader_code{
public:
	static boost::shared_ptr<shader_code> create( std::string const& code, salviar::languages lang, std::vector<std::string>& results );
	static boost::shared_ptr<shader_code> create_and_log(std::string const& code, salviar::languages lang);

	virtual shader_abi const* abii() const = 0;
	virtual void abii( boost::shared_ptr<shader_abi> const& ) = 0;
	virtual void  update_native_function() = 0;
	virtual void* function_pointer() const = 0;
};

END_NS_SALVIAR();

#endif