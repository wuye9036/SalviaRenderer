#include <sasl/include/driver/options.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/bind.hpp>
#include <eflib/include/platform/boost_end.h>

using boost::to_lower;

using std::locale;
using std::pair;
using std::string;
using std::vector;

BEGIN_NS_SASL_DRIVER();

// options filter
void options_filter::reg_extra_parser( po::basic_command_line_parser<char>& ){
}

//////////////////////////////////////////////////////////////////////////
// display info

const char* options_display_info::version_tag = "version,v";
const char* options_display_info::version_desc = "Show version and copyright information";
const char* options_display_info::version_info = 
	"SALVIA Shading Language Compiler 0.4 \r\n"
	"Copyright (C) 2007-2012 SALVIA Development Group."
	"This software and its full source code copyright is GPLv2.";

const char* options_display_info::help_tag = "help,h";
const char* options_display_info::help_desc = "Display this information.";

options_display_info::options_display_info()
	: show_help(false), show_version(false)
{}

void options_display_info::fill_desc( po::options_description& desc )
{
	desc.add_options()
		( help_tag, help_desc)
		( version_tag, version_desc )
		;
	pdesc = &desc;
}

void options_display_info::filterate( po::variables_map const & vm )
{
	show_help		= ( vm.empty() || vm.count("help") > 0 );
	show_version	= ( vm.count("version") > 0 );
}

//////////////////////////////////////////////////////////////////////////
// input & output

const char* options_io::in_tag = "input,i";
const char* options_io::in_desc = "Specify input files.";

const char* options_io::out_tag = "out,o";
const char* options_io::out_desc = "File name of output.";

char const* options_io::dump_ir_tag = "dump_ir";
char const* options_io::dump_ir_desc = "Dump LLVM IR to file";

const char* options_io::export_as_tag = "export-as";
const char* options_io::export_as_desc = "Specifies the content of output file that the compiler should generate.";

const char* options_io::lang_tag = "lang";
const char* options_io::lang_desc = "Specifies language the input file was treated as.'general(g)','vertex_shader(vs)','pixel_shader(ps)','blend_shader(bs)' are available. ";

options_io::options_io() : fmt(none), lang(salviar::lang_none)
{
}

void options_io::fill_desc( po::options_description& desc )
{
	desc.add_options()
		( in_tag, po::value< vector<string> >(&in_names), in_desc )
		( out_tag, po::value< string >(&output_name), out_desc )
		( dump_ir_tag, po::value< string >(&dump_ir), dump_ir_desc )
		( export_as_tag, po::value< string >(&fmt_str), export_as_desc )
		( lang_tag, po::value< string >(&lang_str), lang_desc )
		;
}

void options_io::filterate( po::variables_map const & vm )
{
	if( !vm.count("out") ){
		// TODO Guess output from input.
	}

	if( !vm.count("export-as") ){
		fmt = llvm_ir;
	} else {
		to_lower(fmt_str);
		if( fmt_str == "llvm" || fmt_str == "llvm_ir" ){
			fmt = llvm_ir;
		}
	}

	if ( !vm.count("lang") ){
		lang = salviar::lang_none;
	} else {
		to_lower( lang_str );
		if ( lang_str == "general" || lang_str == "g" ){
			lang = salviar::lang_general;
		} else if ( lang_str == "vertex_shader" || lang_str == "vs" ){
			lang = salviar::lang_vertex_shader;
		} else if ( lang_str == "pixel_shader" || lang_str == "ps" ){
			lang = salviar::lang_pixel_shader;
		} else if ( lang_str == "blend_shader" || lang_str == "bs" ){
			lang = salviar::lang_blending_shader;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// options global
void options_global::fill_desc( po::options_description& desc )
{
	desc.add_options()
		(
		"detail-level", po::value<string>(&detail_str),
		"Specify the detail level of compiler output."
		"The optional items are: quite(q), brief(b), normal(n), verbose(v)."
		"Default is normal"
		);
}

void options_global::filterate( po::variables_map const & vm )
{
	detail = normal;
	if( vm.count("detail-level") ){

		to_lower( detail_str );

		if( detail_str == "quite" || detail_str == "q" ){
			detail = quite;
		} else if( detail_str == "brief" || detail_str == "b" ){
			detail = brief;
		} else if( detail_str == "normal" || detail_str == "n" ){
			detail = normal;
		} else if( detail_str == "verbose" || detail_str == "v" ){
			detail = verbose;
		} else if( detail_str == "debug" || detail_str == "d" ){
			detail = debug;
		}

	}
}

// options_predefinition

const char* options_predefinition::define_tag = "define,D";
const char* options_predefinition::define_desc = "-D<name><=><text>, Define macros.";

options_predefinition::options_predefinition(){
	return;
}

void options_predefinition::reg_extra_parser( po::basic_command_line_parser<char>& cmdpar ){
	cmdpar.extra_parser( boost::bind( &options_predefinition::parse_predef, this, _1 ) );
}

void options_predefinition::fill_desc( po::options_description& desc ){
	desc.add_options()
		( define_tag, define_desc )
		;
}

void options_predefinition::filterate( po::variables_map const & vm ){
	// Do nothing. parse_predef will hook all legal definitions.
	return;
}

pair<string, string> options_predefinition::parse_predef( string const& str )
{
	pair<string, string> null_ret( string(""), string("") );

	if( str.find("-D") != 0 ){ return null_ret; }
	if( str.length() <= 2 ){ return null_ret; }

	// Split cmd
	string::const_iterator equal_it = find(str.begin(), str.end(), '=');

	string::const_iterator beg_it = str.begin()+2;
	if( isalpha( *beg_it, locale() ) || *beg_it == '_' ){
		// Check define name
		for( string::const_iterator it = beg_it; it != equal_it; ++it ){
			if( !isalnum(*beg_it) && *beg_it != '_' ){
				return null_ret;
			}
		}
		string name( beg_it, equal_it );

		// Split define content
		string content;
		if( equal_it != str.end() ){
			string::const_iterator content_it = equal_it + 1;
			if( 
				*content_it == '"'
				&& content_it+1 != str.end()
				&& *str.rbegin() == '"'
				)
			{
				content.assign( content_it+1, str.end()-1 );
			} else {
				content.assign( content_it, str.end() );
			}
		}

		defs.push_back( make_pair(name, content) );
		return defs.back();

	} else {
		return null_ret;
	}
}
END_NS_SASL_DRIVER();

