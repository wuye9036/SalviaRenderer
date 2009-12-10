#ifndef SASL_SYNTAX_TREE_EXPRESSION_H
#define SASL_SYNTAX_TREE_EXPRESSION_H

#include "../syntax_tree/node.h"
#include "../../enums/operators.h"
#include <boost/variant.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/fusion/sequence.hpp>
#include <boost/fusion/include/vector.hpp>

struct constant;

struct primary_expression: public node{
	boost::variant< boost::shared_ptr<constant> > value;
};

struct binary_expression: public node{
	binary_expression& operator = ( const boost::fusion::vector<operators, constant, constant>& rhs );
	binary_expression& operator = ( const binary_expression& rhs );

	operators op;
	boost::shared_ptr<constant> left_expr;
	boost::shared_ptr<constant> right_expr;

	binary_expression();
protected:
	//shallow copy
	binary_expression( const binary_expression& rhs );
	
	//inhertied
	node* clone_impl() const;
	node* deepcopy_impl() const;
};

#endif //SASL_SYNTAX_TREE_EXPRESSION_H