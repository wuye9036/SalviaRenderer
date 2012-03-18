#ifndef SASL_DRIVER_DRIVER_H
#define SASL_DRIVER_DRIVER_H

#include <sasl/include/driver/driver_forward.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

namespace sasl
{
	namespace common
	{
		class code_source;
		class diag_chat;
	}
	namespace semantic
	{
		class module_si;
	}
	namespace code_generator
	{
		class codegen_context;
		class jit_engine;
	}
	namespace syntax_tree
	{
		struct node;
	}
}

BEGIN_NS_SASL_DRIVER();

class driver{
public:
	virtual void set_parameter( int argc, char** argv )				= 0;
	virtual void set_parameter( std::string const& cmd )			= 0;

	virtual void set_code_source( boost::shared_ptr<sasl::common::code_source> const& )	= 0;
	virtual void set_code       ( std::string const& code_text )	= 0;
	virtual void set_code_file  ( std::string const& code_file )	= 0;
	virtual void set_diag_chat( sasl::common::diag_chat* diags )	= 0;

	virtual void compile()											= 0;
	virtual boost::shared_ptr< sasl::code_generator::jit_engine > create_jit() = 0;

	virtual boost::shared_ptr< sasl::semantic::module_si >				mod_si() const		= 0;
	virtual boost::shared_ptr< sasl::code_generator::codegen_context>	mod_codegen() const	= 0;
	virtual boost::shared_ptr< sasl::syntax_tree::node >				root() const		= 0;

	virtual ~driver(){}
};

END_NS_SASL_DRIVER();

#endif