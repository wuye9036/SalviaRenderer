#ifndef SASL_PARSER_DETAIL_EXPRESSION_H
#define SASL_PARSER_DETAIL_EXPRESSION_H

#include "../grammars/expression.h"
#include "../../parser_tree/expression.h"
#include <boost/spirit/include/lex_lexertl.hpp>

template <typename IteratorT, typename LexerT>
template <typename TokenDefT>
expression_grammar<IteratorT, LexerT>::expression_grammar( const TokenDefT& tok )
: expression_grammar::base_type( expr )
{
	ppmexpr.reset(new primary_expression_grammar<IteratorT, LexerT>(tok, *this) );
	pcastexpr.reset(new cast_expression_grammar<IteratorT, LexerT>(tok, *ppmexpr, *this) );
	plorexpr.reset(new binary_expression_grammar<IteratorT, LexerT>(tok, *pcastexpr) );

	binary_expression_grammar<IteratorT, LexerT>& lorexpr(*plorexpr);

	expr %= exprlst.alias();

	exprlst %= assignexpr >> *( comma > assignexpr );
	assignexpr %= rhsexpr >> *( opassign > rhsexpr );
	rhsexpr %= condexpr | lorexpr;

	condexpr %= lorexpr >> ( question > expr > colon > assignexpr );

	opassign %= tok.marktok_equal | tok.optok_arith_assign;
	comma %= tok.marktok_comma;
	question %= tok.marktok_question;
	colon %= tok.marktok_colon;
}

#endif