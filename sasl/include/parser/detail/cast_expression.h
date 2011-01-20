#ifndef SASL_PARSER_DETAIL_CAST_EXPRESSION_H
#define SASL_PARSER_DETAIL_CAST_EXPRESSION_H

#include <sasl/include/parser/grammars/expression.h>
#include <sasl/include/parser/grammars/token.h>
#include <boost/spirit/include/lex_lexertl.hpp>

template <typename IteratorT, typename LexerT>
template <typename TokenDefT, typename SASLGrammarT>
cast_expression_grammar<IteratorT, LexerT>::cast_expression_grammar( const TokenDefT& tok, SASLGrammarT& g)
:base_type( castexpr, "cast expression grammar" )
{
	// init
	g.cast_expr(*this);

	// grammar
	primary_expression_grammar<IteratorT, LexerT>& pmexpr = g.primary_expr();
	expression_grammar<IteratorT, LexerT>& expr = g.expr();

	// non-terminators
	castexpr %= unaryexpr | typecastedexpr;
	typecastedexpr %= lparen >> ident >> rparen >> expr;

	unaryexpr %= postexpr | unariedexpr;
	unariedexpr = (opinc | opunary) >> castexpr;

	postexpr %= pmexpr >> *( idxexpr | callexpr | memexpr | opinc );

	idxexpr %= lsbracket >> expr >> rsbracket;
	callexpr %=	lparen >> -expr >> rparen;
	memexpr %= opmember > ident;

	// terminators
	ident %= tok.littok_ident;
	lparen %= tok.marktok_lparen;
	rparen %= tok.marktok_rparen;
	opinc %= tok.optok_self_incr;
	opmember %= tok.marktok_dot;
	opunary %= tok.marktok_plus | tok.marktok_minus | tok.marktok_tilde | tok.marktok_exclamation;
	lsbracket %= tok.marktok_lsbracket;
	rsbracket %= tok.marktok_rsbracket;

	// for debug
	castexpr.name("type cast expression");
	typecastedexpr.name("type casted expression");
	unaryexpr.name("unary expression");
	unariedexpr.name("unaried expression");
	postexpr.name("postfix expression");
	idxexpr.name("index expression");
	callexpr.name("functor call expression");
	memexpr.name("member expression");

	ident.name("identfier");
	lparen.name("(");
	rparen.name(")");
	opinc.name("++/--");
	opmember.name(".");
	opunary.name("+/-/!/~");
	lsbracket.name("[");
	rsbracket.name("]");
}

#endif