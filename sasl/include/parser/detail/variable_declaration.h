#ifndef SASL_PARSER_DETAIL_VARIABLE_DECLARATION_H
#define SASL_PARSER_DETAIL_VARIABLE_DECLARATION_H

#include <sasl/include/parser/grammars/declaration.h>
#include <sasl/include/parser/grammars/initialized_declarator_list.h>
#include <boost/spirit/include/lex_lexertl.hpp>

template<typename IteratorT, typename LexerT>
template<typename TokenDefT, typename SASLGrammarT>
variable_declaration_grammar<IteratorT, LexerT>::variable_declaration_grammar(
	const TokenDefT& tok, SASLGrammarT& g)
	: base_type( vardecl, "variable declaration" )
{
	// initialize
	g.vardecl(*this);

	// grammar
	declaration_specifier_grammar< IteratorT, LexerT >& declspec = g.decl_spec();
	initialized_declarator_list_grammar< IteratorT, LexerT >& decllist = g.initdecl_list();

	// non-terminators
	vardecl %= declspec >> decllist;
	
	// terminators
	semicolon %= tok.marktok_semicolon;

	// for debug
	vardecl.name("variable declaration");

	semicolon.name(";");
}

#endif