#include "../../include/syntax_tree/operator_literal.h"

void operator_literal::update()
{
	if ( tok->lit == "+" ){
		op = operators::add;
	}
}