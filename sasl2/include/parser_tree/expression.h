#ifndef SASL_PARSER_TREE_EXPRESSION_H
#define SASL_PARSER_TREE_EXPRESSION_H

#include "parser_tree_forward.h"
#include "literal.h"
#include <boost/fusion/adapted.hpp>

BEGIN_NS_SASL_PARSER_TREE()

struct binary_expression {
	constant expr0;
	operator_literal op;
	constant expr1;
};

END_NS_SASL_PARSER_TREE()

/////////////////////////////////////////////////////////
//  BOOST_FUSION_ADAPT_STRUCT 需要写在全局作用域中  //
/////////////////////////////////////////////////////////
BOOST_FUSION_ADAPT_STRUCT( sasl::parser_tree::binary_expression,
						  ( sasl::parser_tree::constant, expr0 )
						  ( sasl::parser_tree::operator_literal, op )
						  ( sasl::parser_tree::constant, expr1)
						  );

#endif