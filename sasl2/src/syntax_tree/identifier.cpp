#include "../../include/syntax_tree/identifier.h"

using namespace boost;

void identifier::update(){
	ident = tok->lit;
}