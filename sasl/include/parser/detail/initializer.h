#ifndef SASL_PARSER_DETAIL_INITIALIZER_H
#define SASL_PARSER_DETAIL_INITIALIZER_H

#include <sasl/include/parser/grammars/initializer.h>
#include <sasl/include/parser/grammars/expression.h>
#include <boost/spirit/include/lex_lexertl.hpp>

template<typename IteratorT, typename LexerT>
template<typename TokenDefT, typename SASLGrammarT>
initializer_grammar<IteratorT, LexerT>::initializer_grammar(const TokenDefT& tok, SASLGrammarT& g)
: base_type(start, "initializer"){
	// init
	g.init(*this);

	// grammar
	assignment_expression_grammar<IteratorT, LexerT>& assignexpr = g.assign_expr();

	// non-terminaters rules
	start %= equal > c_style_initializer;
	paren_initializer %= lparen > -(assignexpr >> *(comma > assignexpr)) > rparen;
	c_style_initializer %= 
		assignexpr
		| nullable_initlst;
	nullable_initlst %= lbrace > -initializer_list > rbrace;
	initializer_list %= c_style_initializer >> *(comma > c_style_initializer);

	// terminators
	lbrace %= tok.marktok_lbrace;
	rbrace %= tok.marktok_rbrace;
	comma %= tok.marktok_comma;
	lparen %= tok.marktok_lparen;
	rparen %= tok.marktok_rparen;
	equal %= tok.marktok_equal;

	// for debug
	start.name("initializer");
	paren_initializer.name("paren initializer");
	c_style_initializer.name("C-style initializer");
	nullable_initlst.name("nullable initializer list");
	initializer_list.name("initializer list");

	lbrace.name("{");
	rbrace.name("}");
	comma.name(",");
	lparen.name("(");
	rparen.name(")");
	equal.name("=");
}

#endif