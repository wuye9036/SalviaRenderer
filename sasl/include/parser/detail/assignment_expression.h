#ifndef SASL_PARSER_DETAIL_ASSIGNMENT_EXPRESSION_H
#define SASL_PARSER_DETAIL_ASSIGNMENT_EXPRESSION_H

#include "../grammars/expression.h"
#include "../grammars/token.h"
#include <boost/spirit/include/lex_lexertl.hpp>

template<typename IteratorT, typename LexerT>
template<typename TokenDefT, typename SASLGrammarT>
assignment_expression_grammar<IteratorT, LexerT>::assignment_expression_grammar(const TokenDefT& tok, SASLGrammarT& g)
:base_type( assignexpr ){
	// init
	g.assign_expr(*this);

	// grammars
	expression_grammar<IteratorT, LexerT>& expr = g.expr();
	binary_expression_grammar<IteratorT, LexerT>& lorexpr = g.bin_expr();

	// non-terminators
	assignexpr %= rhsexpr >> *( opassign > rhsexpr );
	rhsexpr %= condexpr | lorexpr;
	condexpr %= lorexpr >> ( question > expr > colon > assignexpr );

	// terminators
	opassign %= tok.marktok_equal | tok.optok_arith_assign;
	question %= tok.marktok_question;
	colon %= tok.marktok_colon;

	// for debug
	assignexpr.name("assignment expression");
	rhsexpr.name("assignment right-handle side expression");
	condexpr.name("?: expression");

	opassign.name("?=");
	question.name("?");
	colon.name(":");
}

#endif