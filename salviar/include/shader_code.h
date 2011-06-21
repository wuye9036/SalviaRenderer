#ifndef SALVIAR_SHADER_CODE_H
#define SALVIAR_SHADER_CODE_H

#include <salviar/include/salviar_forward.h>

#include <salviar/include/shader.h>

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
	shader_code();

	virtual sasl::semantic::abi_info const* abii() const;
	virtual void abii( boost::shared_ptr<sasl::semantic::abi_info> const& );
	
	void update();
	virtual void* function_pointer() const;
	
	virtual void jit( boost::shared_ptr<sasl::code_generator::jit_engine> const&  );
	
private:
	boost::shared_ptr<sasl::semantic::abi_info> shader_abii;
	boost::shared_ptr<sasl::code_generator::jit_engine> je;
	void* pfn;
};

END_NS_SALVIAR();

extern "C"{
	void salvia_create_shader( boost::shared_ptr<salviar::shader_code>& , std::string const& code, salviar::languages lang );
}
#endif