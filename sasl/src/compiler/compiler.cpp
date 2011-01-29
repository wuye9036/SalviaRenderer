
#include <eflib/include/platform/boost_begin.h>
#include <boost/foreach.hpp>
#include <boost/program_options.hpp>
#include <eflib/include/platform/boost_end.h>

#include <iostream>

namespace po = boost::program_options;

using std::cout;
using std::endl;

const char* version_tag = "version,v";
const char* version_desc = "Show version and copyright information";
const char* version_info = 
	"SoftArt/Salvia Shading Language Compiler(sac) 1.0 pre-alpha\r\n"
	"Copyright (C) 2010 SoftArt/Salvia Development Group."
	"This software and its full source code copyright is GPLv2.";

const char* help_tag = "help,h";
const char* help_desc = "Display this information.";

const char* out_tag = "out,o";
const char* out_desc = "File name of output.";

const char* export_as_tag = "export-as";
const char* export_as_desc = "Specifies the content of output file that the compiler should generate.";

int main (int argc, char **argv){
	po::options_description desc("options");
	desc.add_options()
		( help_tag, help_desc)
		( version_tag, version_desc )
		( out_tag, out_desc )
		( export_as_tag, export_as_desc)
		;

	po::variables_map vm;
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

	if( vm.empty() || vm.count("help") ){
		cout << desc << endl;
	}

	if( vm.count("version") ){
		cout << version_info << endl;
	}

	return 0;
}