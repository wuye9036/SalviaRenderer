#ifndef SASL_DRIVER_DRIVER_API_H
#define SASL_DRIVER_DRIVER_API_H

#include <sasl/include/driver/driver_forward.h>

#if defined(sasl_driver_EXPORTS)
#define SASL_DRIVER_API __declspec(dllexport)
#else
#define SASL_DRIVER_API __declspec(dllimport)
#endif

BEGIN_NS_SASL_DRIVER();

class driver{
public:
	virtual void set_parameter( int argc, char** argv )				= 0;
	virtual void set_parameter( std::string const& cmd )			= 0;
	
	virtual void set_code_source( std::string const& )				= 0;
	virtual void set_code_source( shared_ptr<code_source> const& )	= 0;
	virtual void set_diag_chat( diag_chat* diags )					= 0;

	virtual void compile()											= 0;

	virtual boost::shared_ptr< sasl::semantic::module_si >				mod_si() const		= 0;
	virtual boost::shared_ptr< sasl::code_generator::codegen_context>	mod_codegen() const	= 0;
	virtual boost::shared_ptr< sasl::syntax_tree::node >				root() const		= 0;
};

END_NS_SASL_DRIVER();

extern "C"
{
	boost::shared_ptr<sasl::driver::driver> create_driver();
};

#endif