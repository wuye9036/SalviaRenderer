#ifndef SASL_PARSER_BINARY_EXPRESSION_H
#define SASL_PARSER_BINARY_EXPRESSION_H

#include "parser_forward.h"
#include "../syntax_tree/constant.h"
#include "../syntax_tree/expression.h"
#include "../syntax_tree/operator_literal.h"
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/lex_lexertl.hpp>

DEFINE_GRAMMAR( binary_expression_grammar, binary_expression() ){
	template <typename TokenDefT>
	binary_expression_grammar( const TokenDefT& tok ): binary_expression_grammar::base_type( start )
	{
		using boost::spirit::qi::_val;
		using boost::spirit::qi::_1;

		start = pre_start [_val = _1];

		pre_start %= (  literal_int >> literal_op >> literal_int );

		literal_int = 
			tok.littok_int [_val = _1]
			;

		literal_op = 
			tok.optok_add [_val = _1]
			;
	}

	RULE_DEFINE_HELPER();

	typename rule<operator_literal()>::type literal_op;
	typename rule<constant()>::type literal_int;
	typename rule<boost::fusion::vector<constant, operator_literal, constant>()>::type pre_start;
	typename rule<binary_expression()>::type start;
};

#endif // SASL_PARSER_BINARY_EXPRESSION_H