#ifndef SASL_PARSER_GRAMMARS_EXPRESSION_H
#define SASL_PARSER_GRAMMARS_EXPRESSION_H

#include "../parser_forward.h"
#include "../../parser_tree/expression.h"
#include "../../parser_tree/literal.h"
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/lex.hpp>

DEFINE_GRAMMAR( binary_expression_grammar, sasl::parser_tree::binary_expression() ){
	template <typename TokenDefT>
	binary_expression_grammar( const TokenDefT& tok );

	RULE_DEFINE_HELPER();

	typename rule<sasl::parser_tree::binary_expression()>::type expr;

	typename rule<sasl::parser_tree::binary_expression()>::type binexpr;
	typename rule<sasl::parser_tree::primary_expression()>::type pmexpr;
	typename rule<sasl::parser_tree::paren_expression()>::type parenexpr;

	typename rule<sasl::parser_tree::operator_literal()>::type literal_op, literal_lparen, literal_rparen;

	typename rule<sasl::parser_tree::constant()>::type literal_int;
};

#endif