#ifndef SASL_SYNTAX_TREE_SYNTAX_TREE_BUILDER_H
#define SASL_SYNTAX_TREE_SYNTAX_TREE_BUILDER_H

#include "constant.h"
#include "operator_literal.h"
#include "expression.h"

#include "../parser_tree/literal.h"
#include "../parser_tree/expression.h"

struct syntax_tree_builder{
	binary_expression::handle_t build_expression( const sasl::parser_tree::binary_expression& rhs ){
		binary_expression* ret = new binary_expression();
		ret->left_expr = build_constant( rhs.expr0 );
		ret->right_expr = build_constant( rhs.expr1 );
		ret->op = build_op( rhs.op );

		return binary_expression::handle_t( ret );
	}

	operator_literal::handle_t build_op( const sasl::parser_tree::operator_literal& rhs ){
		operator_literal* ret = new operator_literal();
		ret->tok.reset( new token_attr(rhs) );
		return operator_literal::handle_t(ret);
	}

	constant::handle_t build_constant( const sasl::parser_tree::constant& rhs ){
		constant* ret = new constant();
		ret->tok.reset( new token_attr(rhs) );
		return constant::handle_t(ret);
	}

};

#endif