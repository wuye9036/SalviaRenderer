#include "declaration.h"

declaration::declaration(){
}

void instantiate_declaration(){
	ast_parse( (char*)(NULL), declaration(), white_space() );
}