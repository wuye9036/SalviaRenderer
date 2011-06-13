#ifndef SALVIAR_SHADER_CODE_H
#define SALVIAR_SHADER_CODE_H

#include <salviar/include/salviar_forward.h>

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
	virtual sasl::semantic::abi_info const* abii() const;
	virtual void abii( sasl::semantic::abi_info const* );
	
	virtual void* function_pointer() const;

	virtual void jit( boost::shared_ptr<sasl::code_generator::jit_engine> const&  );
	virtual boost::shared_ptr<sasl::code_generator::jit_engine> jit() const;
private:
	boost::shared_ptr<sasl::code_generator::jit_engine> je;
};

END_NS_SALVIAR();

#endif