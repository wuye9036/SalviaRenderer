#ifndef SASL_DRIVER_DRIVER_IMPL_H
#define SASL_DRIVER_DRIVER_IMPL_H

#include <sasl/include/driver/driver_forward.h>
#include <sasl/include/driver/driver.h>
#include <sasl/include/driver/options.h>

BEGIN_NS_SASL_DRIVER();
 
class driver_impl: public driver{
public:
	driver_impl();

	// All setting functions must be called before calling compile().
	virtual void set_parameter( int argc, char** argv );
	virtual void set_parameter( std::string const& cmd );

	virtual void set_code       ( std::string const& code_text );
	virtual void set_code_file  ( std::string const& code_file );
	virtual void set_code_source( boost::shared_ptr<sasl::common::code_source> const& );
	virtual void set_lex_context( boost::shared_ptr<sasl::common::lex_context> const& );

	/// Only support by default code_source.
	virtual void add_virtual_file( std::string const& file_name, std::string const& code_content, bool high_priority );
	/// Only support by default code_source.
	virtual void set_include_handler( include_handler_fn inc_handler );
	/// Only support by default code source.
	virtual void add_include_path( std::string const& inc_path );
	/// Only support by default code source.
	virtual void add_sysinclude_path( std::string const& sys_path );
	/// Only support by default code source.
	virtual void clear_sysinclude_paths();
	/// Only support by default code source.
	virtual void add_macro( std::string const& macro, bool predef );
	/// Only support by default code source.
	virtual void remove_macro( std::string const& macro );
	/// Only support by default code source.
	virtual void clear_macros();

	virtual void set_diag_chat( sasl::common::diag_chat* diags );
	// virtual void set_dump_ir( std::string const& );

	virtual void compile();

	virtual boost::shared_ptr<sasl::code_generator::jit_engine> create_jit();
	virtual boost::shared_ptr<sasl::code_generator::jit_engine> create_jit( external_function_array const& );

	virtual boost::shared_ptr<sasl::semantic::module_si>			 mod_si() const;
	virtual boost::shared_ptr<sasl::code_generator::codegen_context> mod_codegen() const;
	virtual boost::shared_ptr<sasl::syntax_tree::node>				 root() const;
	virtual boost::shared_ptr<sasl::semantic::abi_info>				 mod_abi() const;

	boost::program_options::variables_map const &	variables() const;
	options_display_info const &					display_info() const;
	options_io const &								io_info() const;

private:
	driver_impl( driver_impl const& );
	driver_impl& operator = ( driver_impl const& );

	template <typename ParserT> bool parse( ParserT& parser );
	void inject_function(
		boost::shared_ptr<sasl::code_generator::jit_engine> const& je,
		void* pfn, std::string const& name, bool is_raw_name);

	boost::shared_ptr<sasl::semantic::module_si>			 msi;
	boost::shared_ptr<sasl::code_generator::codegen_context> mcg;
	boost::shared_ptr<sasl::syntax_tree::node>				 mroot;
	boost::shared_ptr<sasl::semantic::abi_info>				 mabi;

	// Options
	options_global			opt_global;
	options_display_info	opt_disp;
	options_io				opt_io;
	option_macros			opt_macros;
	options_includes		opt_includes;

	po::options_description	desc;
	po::variables_map		vm;

	// Overridden options
	boost::shared_ptr<sasl::common::code_source>	user_code_src;
	boost::shared_ptr<sasl::common::lex_context>	user_lex_ctxt;
	sasl::common::diag_chat*						user_diags;

	typedef boost::unordered_map< std::string,
		std::pair<std::string, bool> > virtual_file_dict;
	enum macro_states
	{
		ms_normal,
		ms_predef,
		ms_remove
	};
	std::vector<std::string>	sys_paths, inc_paths;
	std::vector< std::pair<std::string, macro_states> > macros;
	include_handler_fn			user_inc_handler;
	virtual_file_dict			virtual_files;
};

END_NS_SASL_DRIVER();

#endif