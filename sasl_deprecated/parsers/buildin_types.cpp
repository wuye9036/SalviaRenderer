#include "buildin_types.h"

buildin_type::buildin_type(){
}

void instantiate_buildin_type(){
	ast_parse( (char*)(NULL), buildin_type(), white_space() );
}