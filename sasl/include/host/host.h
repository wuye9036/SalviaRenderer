#ifndef SASL_HOST_HOST_H
#define SASL_HOST_HOST_H

#include <sasl/include/host/host_forward.h>

#include <salviar/include/shader_code.h>

BEGIN_NS_SASL_HOST();

class shader_code_impl: public salviar::shader_code{
public:
	shader_code_impl();

	virtual salviar::shader_abi const* abii() const;
	virtual void abii( boost::shared_ptr<salviar::shader_abi> const& );
	virtual void register_function( void* fnptr, std::string const& name );

	virtual void update_native_function();
	virtual void* function_pointer() const;

	virtual void jit( boost::shared_ptr<sasl::code_generator::jit_engine> const&  );

private:
	boost::shared_ptr<sasl::semantic::abi_info> abi;
	boost::shared_ptr<sasl::code_generator::jit_engine> je;
	void* pfn;
};

END_NS_SASL_HOST();

extern "C"{
	SASL_HOST_API void salvia_create_shader( boost::shared_ptr<salviar::shader_code>& , std::string const& code, salviar::languages lang );
};

#endif