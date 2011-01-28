
#include <eflib/include/platform/boost_begin.h>
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

int main (int argc, char **argv){
	po::options_description desc("options");
	desc.add_options()
		( help_tag, help_desc)
		( version_tag, version_desc )
		;

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);    

	if( vm.empty() || vm.count("help") ){
		cout << desc << endl;
	}

	if( vm.count(version_tag) ){
		cout << version_info << endl;
	}

	return 0;
}