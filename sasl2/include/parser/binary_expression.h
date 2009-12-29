#ifndef SASL_PARSER_BINARY_EXPRESSION_H
#define SASL_PARSER_BINARY_EXPRESSION_H

#include "parser_forward.h"

#include "../parser_tree/literal.h"
#include "../parser_tree/expression.h"

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/lex_lexertl.hpp>

DEFINE_GRAMMAR( binary_expression_grammar, sasl::parser_tree::binary_expression() ){
	template <typename TokenDefT>
	binary_expression_grammar( const TokenDefT& tok ): binary_expression_grammar::base_type( start )
	{
		start %= ( literal_int >> literal_op >>  literal_int );

		literal_op %= tok.optok_add;
		literal_int %= tok.littok_int;
	}

	RULE_DEFINE_HELPER();

	typename rule<sasl::parser_tree::binary_expression()>::type start;
	typename rule<sasl::parser_tree::operator_literal()>::type literal_op;
	typename rule<sasl::parser_tree::constant()>::type literal_int;
};

#endif // SASL_PARSER_BINARY_EXPRESSION_H