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
		class module_semantic;
	}
	namespace codegen{
		class module_vmcode;
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

	detail_level	detail;
	std::string		detail_str;
};

class options_display_info: public options_filter{
public:
	options_display_info();

	void fill_desc( po::options_description& desc );
	void filterate( po::variables_map const & vm );

	po::options_description* pdesc;

	bool show_help;
	bool show_version;

	static const char* version_info;

private:
	static const char* help_tag;
	static const char* help_desc;

	static const char* version_tag;
	static const char* version_desc;
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
	
	std::string			input_file;
	export_format		fmt;
	std::string			fmt_str;
	salviar::languages	lang;
	std::string			lang_str;
	std::string			output_file_name;
	std::string			dump_ir;

private:
	static const char* in_tag;
	static const char* in_desc;

	static const char* lang_tag;
	static const char* lang_desc;

	static const char* out_tag;
	static const char* out_desc;
	
	static const char* dump_ir_tag;
	static const char* dump_ir_desc;

	static const char* export_as_tag;
	static const char* export_as_desc;
};

class option_macros: public options_filter
{
public:
	option_macros();

	void fill_desc( po::options_description& desc );
	void filterate( po::variables_map const & vm );

	std::vector<std::string> defines, predefs, undefs;

private:
	static const char* define_tag;
	static const char* define_desc;
	static const char* predef_tag;
	static const char* predef_desc;
	static const char* undef_tag;
	static const char* undef_desc;
};

class options_includes: public options_filter
{
public:
	options_includes();

	void fill_desc( po::options_description& desc );
	void filterate( po::variables_map const & vm );

	std::vector<std::string> sysincls, includes;
private:
	static char const* include_tag;
	static char const* include_desc;
	static char const* sysincl_tag;
	static char const* sysincl_desc;
};

END_NS_SASL_DRIVER();

#endif