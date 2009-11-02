#include "unit.h"
#include "project.h"
#include "error_reporter.h"
#include "context.h"

#include "../parsers/parsers.h"

#include <boost/format.hpp>
#include <boost/spirit/include/classic_ast.hpp>

#include <fstream>

using namespace std;
using namespace boost;

unit::unit()
{
	this->herror_reporter_ = shared_ptr<error_reporter>(proj().get_error_reporter()->clone());
	hcontext_ = boost::shared_ptr<context>( new context(this) );
}

context* unit::get_context(){
	return hcontext_.get();
}

scope* unit::get_global_scope()
{
	return hscope_.get();
}

bool unit::parse(const string& filePath){
	fstream fs;
	fs.open(filePath.c_str(), ios::in);
	if (!fs.is_open()){
		herror_reporter_->report_compiler_internal_error(
			str( format("%s\"%s\"%s") % "输入文件" % filePath.c_str() % "无法打开!" )
			);
	}

	hcontext_->set_code_pos( context::code_pos(filePath, 1) );
	std::string source_code; // We will read the contents here.
	fs.unsetf(std::ios::skipws); // No white space skipping!
	std::copy(
		std::istream_iterator<char>(fs),
		std::istream_iterator<char>(),
		std::back_inserter(source_code));
	fs.close();

	return boost::spirit::classic::ast_parse(source_code.c_str(), program(), white_space(this) ).full;
}