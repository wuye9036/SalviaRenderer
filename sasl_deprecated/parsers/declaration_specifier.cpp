#include "declaration_specifier.h"

declaration_specifier::declaration_specifier(){}

void instantiate_declaration_specifier(){
	ast_parse( (char*)(NULL), declaration_specifier(), white_space() );
}