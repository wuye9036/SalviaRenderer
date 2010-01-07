#ifndef SASL_PARSER_DETAIL_BINARY_EXPRESSION_H
#define SASL_PARSER_DETAIL_BINARY_EXPRESSION_H

#include "../grammars/expression.h"
#include "../../parser_tree/expression.h"
#include <boost/spirit/include/lex_lexertl.hpp>

template <typename IteratorT, typename LexerT>
template <typename TokenDefT>
binary_expression_grammar<IteratorT, LexerT>::binary_expression_grammar( const TokenDefT& tok ): binary_expression_grammar::base_type( start )
{
	start %= ( literal_int >> literal_op >>  literal_int );

	literal_op %= tok.optok_add;
	literal_int %= tok.littok_int;
}

#endif // SASL_PARSER_BINARY_EXPRESSION_H