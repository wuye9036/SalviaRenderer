#include "expression_list.h"

expression_list::expression_list(){
}

void instaniate_expression_list(){
	ast_parse( (char*)(NULL), expression_list(), white_space() );
}