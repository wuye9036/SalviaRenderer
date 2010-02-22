#ifndef SASL_PARSER_DETAIL_LITERAL_CONSTANT_H
#define SASL_PARSER_DETAIL_LITERAL_CONSTANT_H

#include "../grammars/literal_constant.h"
#include "../grammars/token.h"
#include <boost/spirit/include/lex_lexertl.hpp>

template <typename IteratorT, typename LexerT>
template <typename TokenDefT, typename SASLGrammarT>
literal_constant_grammar<IteratorT, LexerT>::literal_constant_grammar( const TokenDefT& tok, SASLGrammarT& g )
: base_type( lit_const, "literal constant grammar" )
{
	// init
	g.lit_const(*this);

	// grammar

	// non-terminators

	// terminators
	lit_const %= tok.littok_int | tok.littok_float | tok.littok_bool;

	// for debug
	lit_const.name("literal constant");
}


#endif