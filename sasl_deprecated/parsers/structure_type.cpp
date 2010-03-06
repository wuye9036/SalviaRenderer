#include "structure_type.h"

structure_type::structure_type()
{
}

void instantiate_structure_type(){
	ast_parse( (char*)(NULL), structure_type(), white_space(NULL) );
}