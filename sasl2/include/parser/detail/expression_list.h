#ifndef SASL_PARSER_DETAIL_EXPRESSION_LIST_H
#define SASL_PARSER_DETAIL_EXPRESSION_LIST_H

#include "../grammars/expression.h"
#include <boost/spirit/include/lex_lexertl.hpp>

template <typename IteratorT, typename LexerT>
template <typename TokenDefT, typename SASLGrammarT>
expression_list_grammar<IteratorT, LexerT>::expression_list_grammar(const TokenDefT& tok, SASLGrammarT& g)
:base_type(exprlst, "expression list")
{
	// init
	g.expr_list(*this);

	// grammars
	assignment_expression_grammar<IteratorT, LexerT>& assignexpr = g.assign_expr();

	// non-terminators
	exprlst %= assignexpr >> *( comma > assignexpr );

	// terminators
	comma %= tok.marktok_comma;

	// for debug
	exprlst.name("expression list");
	comma.name(",");
}

#endif