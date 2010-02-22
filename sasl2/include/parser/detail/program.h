#ifndef SASL_PARSER_DETAIL_PROGRAM_H
#define SASL_PARSER_DETAIL_PROGRAM_H

#include "../grammars/program.h"
#include "../grammars/declaration.h"
#include <boost/spirit/include/lex_lexertl.hpp>

template <typename IteratorT, typename LexerT>
template <typename TokenDefT, typename SASLGrammarT>
program_grammar<IteratorT, LexerT>::program_grammar( const TokenDefT& tok, SASLGrammarT& g)
:base_type(prog){
	// init
	g.prog(*this);

	// grammar
	declaration_grammar<IteratorT, LexerT>& decl = g.decl();

	// non termiators
	prog %=	*decl;

	// terminators

	// for debug
	prog.name("program");
}

#endif