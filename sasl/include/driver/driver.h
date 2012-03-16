#ifndef SASL_DRIVER_DRIVER_H
#define SASL_DRIVER_DRIVER_H

#include <sasl/include/driver/driver_forward.h>

BEGIN_NS_SASL_DRIVER();

class driver_impl: public driver{
public:
	driver_impl();

	virtual bool set_parameter( int argc, char** argv )		= 0;
	virtual bool set_parameter( std::string const& cmd )	= 0;

	virtual void set_code_source( std::string const& )				= 0;
	virtual void set_code_source( shared_ptr<code_source> const& )	= 0;
	virtual void set_diag_chat( diag_chat* diags )					= 0;

	virtual void compile()											= 0;

	virtual boost::shared_ptr< sasl::semantic::module_si >				mod_si() const		= 0;
	virtual boost::shared_ptr< sasl::code_generator::codegen_context>	mod_codegen() const	= 0;
	virtual boost::shared_ptr< sasl::syntax_tree::node >				root() const		= 0;
};

END_NS_SASL_DRIVER();

#endif