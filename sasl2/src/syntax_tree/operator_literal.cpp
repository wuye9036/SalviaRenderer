#include "../../include/syntax_tree/operator_literal.h"

operator_literal::operator_literal():
node( syntax_node_types::node, token_attr() ), op( operators::none )
{
}

operator_literal::operator_literal(const operator_literal &rhs)
: op(rhs.op), node(rhs){
}

operator_literal& operator_literal::operator =(const operator_literal &rhs){
	op = rhs.op;
	tok = rhs.tok;
	return *this;
}

operator_literal& operator_literal::operator =(const token_attr &tok){
	this->tok = tok;
	if( tok.lit == "+" ){
		op = operators::add;
	}
	return *this;
}

node* operator_literal::clone_impl() const{
	return new operator_literal(*this);
}

node* operator_literal::deepcopy_impl() const{
	return new operator_literal(*this);
}