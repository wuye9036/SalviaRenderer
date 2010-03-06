#include "../../include/syntax_tree/operator_literal.h"

void operator_literal::update()
{
	if ( tok->lit == "+" ){
		op = operators::add;
	} else if (tok->lit == "-" ) {
		op = operators::sub;
	} else if (tok->lit == "*" ) {
		op = operators::mul;
	} else if (tok->lit == "/" ) {
		op = operators::div;
	}
}