#include "whitespace.h"
#include "declaration.h"

using namespace std;

white_space::white_space(unit* punit): punit_(punit){
}

//for instantiate grammar template. not execute.
void instantiate_whitespace(){
	ast_parse( (char*)(NULL), declaration(), white_space() );
}