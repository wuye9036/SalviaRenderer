#ifndef SASL_PARSER_BINARY_EXPRESSION_H
#define SASL_PARSER_BINARY_EXPRESSION_H

#include "parser_forward.h"
#include "../syntax_tree/constant.h"
#include "../syntax_tree/expression.h"
#include "../syntax_tree/operator_literal.h"
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/lex_lexertl.hpp>

DEFINE_GRAMMAR( binary_expression_grammar, binary_expression::handle_t() ){
	template <typename TokenDefT>
	binary_expression_grammar( const TokenDefT& tok ): binary_expression_grammar::base_type( start )
	{
		start %= ( literal_int >> literal_op >>  literal_int );

		literal_op %= tok_op;
		literal_int %= tok_int;

		tok_op %= tok.optok_add;
		tok_int %= tok.littok_int;
	}

	RULE_DEFINE_HELPER();

	typename rule<binary_expression::handle_t()>::type start;
	typename rule<operator_literal::handle_t()>::type literal_op;
	typename rule<constant::handle_t()>::type literal_int;

	typename rule<token_attr::handle_t()>::type tok_op, tok_int;
};

#endif // SASL_PARSER_BINARY_EXPRESSION_H