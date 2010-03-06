#include "initialized_declarator.h"

initialized_declarator::initialized_declarator(){
}

void instantiate_initialized_declarator(){
	ast_parse( (char*)(NULL), initialized_declarator(), white_space() );
}