#ifndef SASL_DRIVER_OPTIONS_H
#define SASL_DRIVER_OPTIONS_H

#include <sasl/include/driver/driver_forward.h>
#include <sasl/include/syntax_tree/parse_api.h>

#include <salviar/include/shader.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/program_options.hpp>
#include <eflib/include/platform/boost_end.h>

#include <string>
#include <vector>

namespace sasl{
	namespace syntax_tree{
		struct node;
	}
	namespace semantic{
		class module_si;
	}
	namespace code_generator{
		class codegen_context;
	}
}

BEGIN_NS_SASL_DRIVER();

namespace po = boost::program_options;

class compiler;

class options_filter{
public:
	virtual void reg_extra_parser( po::basic_command_line_parser<char>& );
	virtual void fill_desc( po::options_description& desc ) = 0;
	virtual void filterate( po::variables_map const& vm ) = 0;
};

class options_global: public options_filter{
public:
	void fill_desc( po::options_description& desc );
	void filterate( po::variables_map const & vm );

	enum detail_level{
		none,
		quite,
		brief,
		normal,
		verbose,
		debug
	};

	detail_level detail() const;

private:
	detail_level detail_lvl;
	std::string detail_lvl_str;
};

class options_display_info: public options_filter{
public:
	options_display_info();

	void fill_desc( po::options_description& desc );
	void filterate( po::variables_map const & vm );

	bool help_enabled() const;
	bool version_enabled() const;

	char const* version() const;

private:
	static const char* version_tag;
	static const char* version_desc;
	static const char* version_info;

	static const char* help_tag;
	static const char* help_desc;

	po::options_description* pdesc;

	bool h;
	bool v;
};

class options_io: public options_filter{
public:
	options_io();

	void fill_desc( po::options_description& desc );
	void filterate( po::variables_map const & vm );

	enum export_format{
		none,
		llvm_ir
	};
	
	export_format format() const;
	std::vector<std::string> inputs() const;
	salviar::languages language() const;
	std::string output() const;

private:
	export_format fmt;
	std::string fmt_str;
	salviar::languages lang;
	std::string lang_str;
	std::vector< std::string > in_names;
	std::string out_name;

	static const char* in_tag;
	static const char* in_desc;

	static const char* lang_tag;
	static const char* lang_desc;

	static const char* out_tag;
	static const char* out_desc;

	static const char* export_as_tag;
	static const char* export_as_desc;
};

class options_predefinition: options_filter
{
public:
	options_predefinition();

	void reg_extra_parser( po::basic_command_line_parser<char>& );
	void fill_desc( po::options_description& desc );
	void filterate( po::variables_map const & vm );

private:
	std::pair<std::string, std::string> parse_predef( std::string const& );

	std::vector< std::pair<std::string, std::string> > defs;

	static const char* define_tag;
	static const char* define_desc;
};

END_NS_SASL_DRIVER();

#endif