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

struct constant;
struct operator_literal;

struct primary_expression: public node{
	boost::variant< boost::shared_ptr<constant> > value;
};

struct binary_expression: public node{
	binary_expression& operator = ( const boost::fusion::vector< constant, operator_literal, constant > & rhs ){
		using namespace boost;
		using namespace boost::fusion;

		left_expr.reset( at_c<0>(rhs).clone<constant>() );
		op = at_c<1>(rhs).op;
		right_expr.reset( at_c<2>(rhs).clone<constant>() );
		return *this;
	}

	binary_expression& operator = ( const binary_expression& rhs );

	operators op;
	boost::shared_ptr<constant> left_expr;
	boost::shared_ptr<constant> right_expr;

	binary_expression();
	binary_expression( const binary_expression& rhs );

protected:
	//inhertied
	node* clone_impl() const;
	node* deepcopy_impl() const;
};

#endif //SASL_SYNTAX_TREE_EXPRESSION_H