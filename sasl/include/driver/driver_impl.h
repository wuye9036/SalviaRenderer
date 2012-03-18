#ifndef SASL_DRIVER_DRIVER_IMPL_H
#define SASL_DRIVER_DRIVER_IMPL_H

#include <sasl/include/driver/driver_forward.h>
#include <sasl/include/driver/driver.h>
#include <sasl/include/driver/options.h>

BEGIN_NS_SASL_DRIVER();

class driver_impl: public driver{
public:
	driver_impl();

	virtual void set_parameter( int argc, char** argv );
	virtual void set_parameter( std::string const& cmd );

	virtual void set_code       ( std::string const& code_text );
	virtual void set_code_file  ( std::string const& code_file );
	virtual void set_code_source( boost::shared_ptr<sasl::common::code_source> const& );
	virtual void set_lex_context( boost::shared_ptr<sasl::common::lex_context> const& );

	virtual void set_diag_chat( sasl::common::diag_chat* diags );
	// virtual void set_dump_ir( std::string const& );

	virtual void compile();

	virtual boost::shared_ptr<sasl::code_generator::jit_engine> create_jit();

	virtual boost::shared_ptr< sasl::semantic::module_si >				mod_si() const;
	virtual boost::shared_ptr< sasl::code_generator::codegen_context>	mod_codegen() const;
	virtual boost::shared_ptr< sasl::syntax_tree::node >				root() const;

	boost::program_options::variables_map const &	variables() const;
	options_display_info const &					display_info() const;
	options_io const &								io_info() const;

private:
	driver_impl( driver_impl const& );
	driver_impl& operator = ( driver_impl const& );

	template <typename ParserT> bool parse( ParserT& parser );

	boost::shared_ptr< sasl::semantic::module_si >				msi;
	boost::shared_ptr< sasl::code_generator::codegen_context>	mcg;
	boost::shared_ptr< sasl::syntax_tree::node >				mroot;

	// Options
	options_global			opt_global;
	options_display_info	opt_disp;
	options_io				opt_io;
	options_predefinition	opt_predef;

	po::options_description	desc;
	po::variables_map		vm;

	// Overridden options
	boost::shared_ptr<sasl::common::code_source>	user_code_src;
	boost::shared_ptr<sasl::common::lex_context>	user_lex_ctxt;
	sasl::common::diag_chat*						user_diags;
};

END_NS_SASL_DRIVER();

#endif