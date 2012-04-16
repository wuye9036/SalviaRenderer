#ifndef SASL_HOST_HOST_H
#define SASL_HOST_HOST_H

#include <sasl/include/host/host_forward.h>

#include <salviar/include/shader_code.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/tuple/tuple.hpp>
#include <eflib/include/platform/boost_end.h>

namespace sasl {
	namespace semantic
	{
		class abi_info;
	}
	namespace code_generator
	{
		class jit_engine;
	}
}

BEGIN_NS_SASL_HOST();

class shader_code_impl: public salviar::shader_code{
public:
	shader_code_impl();

	virtual salviar::shader_abi const* abii() const;
	virtual void abii( boost::shared_ptr<salviar::shader_abi> const& );
	virtual void update_native_function();
	virtual void* function_pointer() const;

	virtual void jit( boost::shared_ptr<sasl::code_generator::jit_engine> const&  );
private:
	boost::shared_ptr<sasl::semantic::abi_info>			abi;
	boost::shared_ptr<sasl::code_generator::jit_engine>	je;
	void* pfn;
};

END_NS_SASL_HOST();

extern "C"{
	SASL_HOST_API void salvia_create_shader(
		boost::shared_ptr<salviar::shader_code>&,
		std::string const& code,
		salviar::languages lang,
		std::vector< boost::tuple<void*, std::string, bool> > const&);
};

#endif