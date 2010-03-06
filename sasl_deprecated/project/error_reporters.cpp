#include "error_reporters.h"
#include <iostream>

using namespace std;

/*error_reporter_tags*/
string error_reporter_tags::compiler_config_error(){
	return "Compiler configuration Error: ";
}

string error_reporter_tags::syntax_error(){
	return "Syntax Error: ";
}

void stdout_error_reporter::report_compiler_internal_error(const string& errContent){
	cout << error_reporter_tags::compiler_config_error() << errContent;
}

void stdout_error_reporter::report_syntax_error(const string& errContent){
	cout << error_reporter_tags::syntax_error() << errContent;
}

stdout_error_reporter* stdout_error_reporter::clone()
{
	return new stdout_error_reporter();
}
