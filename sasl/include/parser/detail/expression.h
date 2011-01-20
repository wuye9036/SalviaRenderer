#ifndef SASL_PARSER_DETAIL_EXPRESSION_H
#define SASL_PARSER_DETAIL_EXPRESSION_H

#include <sasl/include/parser/grammars/expression.h>
#include <boost/spirit/include/lex_lexertl.hpp>

template <typename IteratorT, typename LexerT>
template <typename TokenDefT, typename SASLGrammarT>
expression_grammar<IteratorT, LexerT>::expression_grammar( const TokenDefT& tok, SASLGrammarT& g )
: base_type( expr, "expression grammar" )
{
	// init
	g.expr(*this);

	// grammar
	expression_list_grammar<IteratorT, LexerT>& exprlst = g.expr_list();

	// non-terminators
	expr %= exprlst;

	// terminators

	//for debug
	expr.name("expression");
}

#endif