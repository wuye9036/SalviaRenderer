#ifndef SASL_SYNTAX_TREE_OPERATOR_LITERAL_H
#define SASL_SYNTAX_TREE_OPERATOR_LITERAL_H

#include "node.h"
#include "token.h"
#include "../../enums/operators.h"

struct operator_literal: public node{
	operator_literal();
	operator_literal(const operator_literal& rhs);
	operator_literal& operator = ( const operator_literal& rhs );
	operator_literal& operator = ( const token_attr& tok );

	operators op;
	virtual node* clone_impl() const;
	virtual node* deepcopy_impl() const;
};

#endif //SASL_SYNTAX_TREE_OPERATOR_LITERAL_H