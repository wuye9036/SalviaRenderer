#include "parsers.h"

#include "program.h"

program::program()
{
}

void instantiate_program(){
	ast_parse( (char*)NULL, program(), white_space() );
}