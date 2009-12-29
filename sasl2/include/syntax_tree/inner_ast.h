#ifndef SASL_SYNTAX_TREE_INNER_AST_H
#define SASL_SYNTAX_TREE_INNER_AST_H

#include <boost/fusion/adapted.hpp>

typedef token_attr constant_;
typedef token_attr operator_literal_;

struct binary_expression_ {
	constant_ expr0;
	operator_literal_ op;
	constant_ expr1;
};

BOOST_FUSION_ADAPT_STRUCT( 
						  binary_expression_,
						  ( constant_, expr0 )
						  ( operator_literal_, op )
						  ( constant_, expr1)
						  );

struct syntax_tree_builder{
	binary_expression::handle_t build_expression( const binary_expression_& rhs ){
		binary_expression* ret = new binary_expression();
		ret->left_expr = build_constant( rhs.expr0 );
		ret->right_expr = build_constant( rhs.expr1 );
		ret->op = build_op( rhs.op );

		return binary_expression::handle_t( ret );
	}

	operator_literal::handle_t build_op( const operator_literal_& rhs ){
		operator_literal* ret = new operator_literal();
		ret->tok.reset( new token_attr(rhs) );
		return operator_literal::handle_t(ret);
	}

	constant::handle_t build_constant( const constant_& rhs ){
		constant* ret = new constant();
		ret->tok.reset( new token_attr(rhs) );
		return constant::handle_t(ret);
	}

};

#endif