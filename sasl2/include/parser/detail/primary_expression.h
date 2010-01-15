#ifndef SASL_PARSER_DETAIL_PRIMARY_EXPRESSION_H
#define SASL_PARSER_DETAIL_PRIMARY_EXPRESSION_H

#include "../grammars/literal_constant.h"
#include "../grammars/expression.h"
#include "../../parser_tree/expression.h"
#include <boost/spirit/include/lex_lexertl.hpp>

template <typename IteratorT, typename LexerT>
template <typename TokenDefT>
primary_expression_grammar<IteratorT, LexerT>::primary_expression_grammar(
	const TokenDefT& tok,
	expression_grammar<IteratorT, LexerT>& e )
: primary_expression_grammar::base_type( pmexpr ),
expr(e),
lit_const(tok)
{
	pmexpr %= lit_const | ident | parenexpr;
	parenexpr %=  lparen > expr > rparen;

	ident %= tok.littok_ident;
	lparen %= tok.marktok_lparen;
	rparen %= tok.marktok_rparen;
}

#endif