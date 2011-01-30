#include <sasl/include/compiler/options.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/foreach.hpp>
#include <eflib/include/platform/boost_end.h>

#include <iostream>

using std::cout;
using std::endl;

BEGIN_NS_SASL_COMPILER();

options_manager options_manager::inst;

options_manager& options_manager::instance()
{
	return inst;
}

void options_manager::parse( int argc, char** argv )
{
	po::parsed_options parsed = po::command_line_parser(argc, argv).options( desc ).allow_unregistered().run();
	std::vector<std::string> unrecg = po::collect_unrecognized( parsed.options, po::include_positional );

	if( !unrecg.empty() ){
		cout << "Warning: options ";
		BOOST_FOREACH( std::string const & str, unrecg ){
			cout << str << " ";
		}
		cout << "are invalid. They were ignored." << endl;
	}

	po::store( parsed, vm );
	po::notify(vm);

	disp_info.filterate(vm);
	out_info.filterate(vm);
}

options_manager::options_manager()
{
	disp_info.fill_desc(desc);
	out_info.fill_desc(desc);
}

void options_manager::process( bool& abort )
{
	abort = false;

	disp_info.process(abort);
	if( abort ){ return; }

	out_info.process(abort);
	if( abort ){ return; }
}

po::variables_map const & options_manager::variables() const
{
	return vm;
}

options_display_info const & options_manager::display_info() const
{
	return disp_info;
}

options_output const & options_manager::output_info() const
{
	return out_info;
}

//////////////////////////////////////////////////////////////////////////
// display info

const char* options_display_info::version_tag = "version,v";
const char* options_display_info::version_desc = "Show version and copyright information";
const char* options_display_info::version_info = 
	"SoftArt/Salvia Shading Language Compiler(sac) 1.0 pre-alpha\r\n"
	"Copyright (C) 2010 SoftArt/Salvia Development Group."
	"This software and its full source code copyright is GPLv2.";

const char* options_display_info::help_tag = "help,h";
const char* options_display_info::help_desc = "Display this information.";

options_display_info::options_display_info()
	: h(false), v(false)
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
	h = ( vm.empty() || vm.count("help") > 0 );
	v = ( vm.count("version") > 0 );
}

void options_display_info::process( bool& abort )
{
	if( h ){
		cout << *pdesc << endl;
		abort = true;
		return;
	}

	if( v ){
		cout << version_info << endl;
		abort = false;
	}
}

bool options_display_info::help_enabled() const
{
	return h;
}

bool options_display_info::version_enabled() const
{
	return v;
}

//////////////////////////////////////////////////////////////////////////
// output

const char* options_output::out_tag = "out,o";
const char* options_output::out_desc = "File name of output.";

const char* options_output::export_as_tag = "export-as";
const char* options_output::export_as_desc = "Specifies the content of output file that the compiler should generate.";

options_output::options_output() : fmt(none)
{
}

void options_output::fill_desc( po::options_description& desc )
{
	desc.add_options()
		( out_tag, out_desc )
		( export_as_tag, export_as_desc)
		;
}

void options_output::filterate( po::variables_map const & vm )
{
}

void options_output::process( bool& abort )
{
}

options_output::export_format options_output::format() const
{
	return fmt;
}

std::string options_output::file_name() const
{
	return fname;
}

END_NS_SASL_COMPILER();