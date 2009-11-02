#include "identifier.h"
#include <boost/spirit/include/classic_ast.hpp>

identifier::identifier(){}

void instantiate_identifier(){
	ast_parse( (char*)NULL, identifier(), white_space() );
}