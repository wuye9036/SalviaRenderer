#ifndef SASL_PARSER_DETAIL_CAST_EXPRESSION_H
#define SASL_PARSER_DETAIL_CAST_EXPRESSION_H

#include "../grammars/expression.h"
#include "../../parser_tree/expression.h"
#include <boost/spirit/include/lex_lexertl.hpp>

template <typename IteratorT, typename LexerT>
template <typename TokenDefT>
cast_expression_grammar<IteratorT, LexerT>::cast_expression_grammar(
	const TokenDefT& tok,
	primary_expression_grammar<IteratorT, LexerT>& pmexpr_,
	expression_grammar<IteratorT, LexerT>& expr_): 
cast_expression_grammar::base_type( castexpr ),
pmexpr(pmexpr_),
expr(expr_)
{
	castexpr %= unaryexpr | typecastedexpr;
	typecastedexpr %= lparen > ident > rparen > expr;

	unaryexpr %= postexpr | unariedexpr;
	unariedexpr = (opinc | opunary) > castexpr;

	postexpr %= pmexpr >> *( idxexpr | callexpr | memexpr | opinc );

	idxexpr %= lsbracket > expr > rsbracket;
	callexpr %=	lparen > -expr > rparen;
	memexpr %= opmember > ident;

	ident %= tok.littok_ident;
	lparen %= tok.marktok_lparen;
	rparen %= tok.marktok_rparen;
	opinc %= tok.optok_self_incr;
	opmember %= tok.marktok_dot;
	opunary %= tok.marktok_plus | tok.marktok_minus | tok.marktok_tilde | tok.marktok_exclamation;
	lsbracket %= tok.marktok_lsbracket;
	rsbracket %= tok.marktok_rsbracket;
}

#endif