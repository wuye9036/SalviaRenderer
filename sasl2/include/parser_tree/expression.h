#ifndef SASL_PARSER_TREE_EXPRESSION_H
#define SASL_PARSER_TREE_EXPRESSION_H

#include "parser_tree_forward.h"
#include "literal.h"
#include <boost/fusion/adapted.hpp>

BEGIN_NS_SASL_PARSER_TREE()

struct paren_expression;

typedef boost::variant< constant, boost::recursive_wrapper<paren_expression> > primary_expression;

struct binary_expression {
	typedef std::vector< 
		boost::fusion::vector<operator_literal, primary_expression>
	> expr_list_t;
	primary_expression first_expr;
	expr_list_t follow_exprs;
};

struct paren_expression {
	operator_literal lparen;
	binary_expression expr;
	operator_literal rparen;
};

END_NS_SASL_PARSER_TREE()

/////////////////////////////////////////////////////////
//  BOOST_FUSION_ADAPT_STRUCT 需要写在全局作用域中  //
/////////////////////////////////////////////////////////
BOOST_FUSION_ADAPT_STRUCT( sasl::parser_tree::binary_expression,
						  ( sasl::parser_tree::primary_expression, first_expr )
						  ( sasl::parser_tree::binary_expression::expr_list_t, follow_exprs )
						  );

BOOST_FUSION_ADAPT_STRUCT(sasl::parser_tree::paren_expression, 
						  (sasl::parser_tree::operator_literal, lparen)
						  (sasl::parser_tree::binary_expression, expr)
						  (sasl::parser_tree::operator_literal, rparen)
						  )
#endif