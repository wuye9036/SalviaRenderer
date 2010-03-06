#include "assignment_expression.h"

assignment_expression::assignment_expression(){
}

void instaniate_assignment_expression(){
	ast_parse( (char*)(NULL), assignment_expression(), white_space() );
}