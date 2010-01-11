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

struct expression: public node_impl<expression>{
	expression( syntax_node_types nodetype)
		:node_impl( nodetype, token_attr::handle_t() ){}
};

struct constant_expression: public expression{
	constant::handle_t value;
	void update(){
		value->update();
	}
	void accept( syntax_tree_visitor* visitor ){
		visitor->visit( *this );
	}
	constant_expression(): expression( syntax_node_types::node ){}
};

struct binary_expression: public expression {
	operator_literal::handle_t op;
	expression::handle_t left_expr;
	expression::handle_t right_expr;

	void accept( syntax_tree_visitor* visitor ){
		visitor->visit( *this );
	}

	void update(){
		op->update();
		left_expr->update();
		right_expr->update();
	}

	binary_expression();
};

#endif //SASL_SYNTAX_TREE_EXPRESSION_H