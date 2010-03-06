#include "expression.h"

expression::expression(){
}

void instaniate_expression(){
	ast_parse( (char*)(NULL), expression(), white_space() );
}