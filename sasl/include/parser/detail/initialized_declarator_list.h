#ifndef SASL_PARSER_DETAIL_INITIALIZED_DECLARATOR_LIST_H
#define SASL_PARSER_DETAIL_INITIALIZED_DECLARATOR_LIST_H

#include "../grammars/initialized_declarator_list.h"
#include "../grammars/initializer.h"
#include <boost/spirit/include/lex_lexertl.hpp>

template <typename IteratorT, typename LexerT>
template <typename TokenDefT, typename SASLGrammarT>
initialized_declarator_list_grammar<IteratorT, LexerT>::initialized_declarator_list_grammar(
	const TokenDefT& tok, SASLGrammarT& g)
	: base_type(start, "initialized declarator list grammar") {
	// init
	g.initdecl_list(*this);

	// grammars
	initializer_grammar<IteratorT, LexerT>& init = g.init();

	// non-terminators
	start %= init_declarator >> *(comma > init_declarator);
	init_declarator %= ident >> -init;

	// terminators
	comma %= tok.marktok_comma;
	ident %= tok.littok_ident;

	// for debug
	start.name("initialized declarator list");
	init_declarator.name("initialized declarator");
	
	comma.name(",");
	ident.name("identifier");
}

#endif