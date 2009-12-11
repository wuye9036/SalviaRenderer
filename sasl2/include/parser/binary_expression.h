#ifndef SASL_PARSER_BINARY_EXPRESSION_H
#define SASL_PARSER_BINARY_EXPRESSION_H

#include "../syntax_tree/constant.h"
#include "../syntax_tree/expression.h"
#include "../syntax_tree/operator_literal.h"
#include <boost/spirit/include/qi.hpp>

template <typename IteratorT>
struct binary_expression_grammar: boost::spirit::qi::grammar<IteratorT, binary_expression() >{
	template <typename TokenDefT>
	binary_expression_grammar( const TokenDefT& tok ): binary_expression_grammar::base_type( start )
	{
		using boost::spirit::qi::_val;
		using boost::spirit::qi::_1;
		using boost::spirit::qi::_2;

		start = pre_start [_val = _1];

		pre_start %= (  literal_int >> literal_op >> literal_int );

		literal_int = 
			tok.littok_int [_val = _1]
			;

		literal_op = 
			tok.optok_add [_val = _1]
			;
	}

	boost::spirit::qi::rule<IteratorT, operator_literal() > literal_op;
	boost::spirit::qi::rule<IteratorT, constant() > literal_int;
	boost::spirit::qi::rule<IteratorT, boost::fusion::vector<constant, operator_literal, constant>() > pre_start;
	boost::spirit::qi::rule<IteratorT, binary_expression() > start;
};

#endif // SASL_PARSER_BINARY_EXPRESSION_H