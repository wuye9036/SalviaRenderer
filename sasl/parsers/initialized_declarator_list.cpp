#include "initialized_declarator_list.h"
//#include "whitespace.h"

initialized_declarator_list::initialized_declarator_list(){
}

void instantiate_initialized_declarator_list(){
	ast_parse( (char*)(NULL), initialized_declarator_list(), white_space() );
}