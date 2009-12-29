#ifndef SASL_PARSER_BINARY_EXPRESSION_H
#define SASL_PARSER_BINARY_EXPRESSION_H

#include "parser_forward.h"

#include "../syntax_tree/constant.h"
#include "../syntax_tree/expression.h"
#include "../syntax_tree/operator_literal.h"
#include "../syntax_tree/inner_ast.h"

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/lex_lexertl.hpp>

DEFINE_GRAMMAR( binary_expression_grammar, binary_expression_() ){
	template <typename TokenDefT>
	binary_expression_grammar( const TokenDefT& tok ): binary_expression_grammar::base_type( start )
	{
		start %= ( literal_int >> literal_op >>  literal_int );

		literal_op %= tok.optok_add;
		literal_int %= tok.littok_int;
	}

	RULE_DEFINE_HELPER();

	typename rule<binary_expression_()>::type start;
	typename rule<operator_literal_()>::type literal_op;
	typename rule<constant_()>::type literal_int;
};

#endif // SASL_PARSER_BINARY_EXPRESSION_H