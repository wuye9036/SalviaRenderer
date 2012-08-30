#ifndef SASL_DRIVER_DRIVER_H
#define SASL_DRIVER_DRIVER_H

#include <sasl/include/driver/driver_forward.h>

#include <eflib/include/utility/shared_declaration.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>
#include <eflib/include/platform/boost_end.h>

#include <vector>
#include <string>

namespace sasl
{
	namespace common
	{
		EFLIB_DECLARE_CLASS_SHARED_PTR(diag_chat);
		EFLIB_DECLARE_CLASS_SHARED_PTR(code_source);
	}
	namespace semantic
	{
		class module_semantic;
		class abi_info;
	}
	namespace codegen
	{
		class cgllvm_module;
		class jit_engine;
	}
	namespace syntax_tree
	{
		struct node;
	}
}

BEGIN_NS_SASL_DRIVER();

typedef boost::function<
	bool/*succeed*/ (
	std::string& /*[out]content*/, std::string& /*[out]native file name*/,
	std::string const& /*file name*/, bool /*is system header*/,
	bool /*check only*/ )
> include_handler_fn;

typedef std::vector< boost::tuple<void* /*function pointer*/, std::string /*name*/, bool /*is_raw_name*/> > external_function_array;

class driver{
public:
	virtual void set_parameter( int argc, char** argv )				= 0;
	virtual void set_parameter( std::string const& cmd )			= 0;

	virtual void set_code_source( sasl::common::code_source_ptr const& )	= 0;
	virtual void set_code       ( std::string const& code_text )	= 0;
	virtual void set_code_file  ( std::string const& code_file )	= 0;

	virtual sasl::common::diag_chat_ptr compile()					= 0;
	virtual boost::shared_ptr<sasl::codegen::jit_engine> create_jit() = 0;
	virtual boost::shared_ptr<sasl::codegen::jit_engine> create_jit(external_function_array const&) = 0;
	virtual void add_virtual_file(
		std::string const& file_name,
		std::string const& code_content,
		bool high_priority ) = 0;
	virtual void set_include_handler( include_handler_fn inc_handler ) = 0;

	virtual boost::shared_ptr<sasl::semantic::module_semantic>		module_sem() const		= 0;
	virtual boost::shared_ptr<sasl::codegen::cgllvm_module>	module() const= 0;
	virtual boost::shared_ptr<sasl::syntax_tree::node>				root() const		= 0;
	virtual boost::shared_ptr<sasl::semantic::abi_info>				mod_abi() const	= 0;

	virtual ~driver(){}
};

END_NS_SASL_DRIVER();

#endif