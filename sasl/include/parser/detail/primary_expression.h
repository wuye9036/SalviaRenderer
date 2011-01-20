#ifndef SASL_PARSER_DETAIL_PRIMARY_EXPRESSION_H
#define SASL_PARSER_DETAIL_PRIMARY_EXPRESSION_H

#include <sasl/include/parser/grammars/expression.h>
#include <sasl/include/parser/grammars/literal_constant.h>
#include <sasl/include/parser/grammars/token.h>
#include <boost/spirit/include/lex_lexertl.hpp>

template <typename IteratorT, typename LexerT>
template <typename TokenDefT, typename SASLGrammarT>
primary_expression_grammar<IteratorT, LexerT>::primary_expression_grammar(
	const TokenDefT& tok, SASLGrammarT& g )
: base_type( pmexpr, "primary expression grammar" )
{
	// init
	g.primary_expr(*this);

	// grammar
	literal_constant_grammar<IteratorT, LexerT>& lit_const = g.lit_const();
	expression_grammar<IteratorT, LexerT>& expr = g.expr();

	// non terminators
	pmexpr %= lit_const | ident | parenexpr;
	parenexpr %=  lparen > expr > rparen;

	// terminators
	ident %= tok.littok_ident;
	lparen %= tok.marktok_lparen;
	rparen %= tok.marktok_rparen;

	// for debug
	pmexpr.name("primary expression");
	parenexpr.name("paren expression");
	
	ident.name("identifier");
	lparen.name("(");
	rparen.name(")");
}

#endif