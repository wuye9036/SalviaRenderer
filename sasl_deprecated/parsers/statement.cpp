#include "statement.h"
#include "whitespace.h"
#include <boost/spirit/include/classic_ast.hpp>

statement::statement(){}

void instantiate_statement(){
	ast_parse( (char*)NULL, statement(), white_space() );
}