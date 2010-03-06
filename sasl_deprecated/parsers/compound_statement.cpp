#include "compound_statement.h"
#include <boost/spirit/include/classic_ast.hpp>

compound_statement::compound_statement(){}

void instantiate_compound_statement(){
	ast_parse( (char*)NULL, compound_statement(), white_space() );
}