#ifndef SASL_SYNTAX_TREE_EXPRESSION_H
#define SASL_SYNTAX_TREE_EXPRESSION_H

#include "../syntax_tree/node.h"
#include "../syntax_tree/constant.h"
#include "../syntax_tree/operator_literal.h"
#include "../../enums/operators.h"
#include <boost/variant.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/fusion/sequence.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/fusion/adapted.hpp>

struct constant;
struct operator_literal;

struct binary_expression: public node_impl<binary_expression> {
	operator_literal::handle_t op;
	constant::handle_t left_expr;
	constant::handle_t right_expr;

	void update(){
		op->update();
		left_expr->update();
		right_expr->update();
	}

	binary_expression();
};

#endif //SASL_SYNTAX_TREE_EXPRESSION_H