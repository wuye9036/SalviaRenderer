#ifndef SASL_PARSER_DETAIL_LITERAL_CONSTANT_H
#define SASL_PARSER_DETAIL_LITERAL_CONSTANT_H

#include "../grammars/literal_constant.h"
#include "../../parser_tree/literal.h"
#include <boost/spirit/include/lex_lexertl.hpp>

template <typename IteratorT, typename LexerT>
template <typename TokenDefT>
literal_constant_grammar<IteratorT, LexerT>::literal_constant_grammar( const TokenDefT& tok )
: literal_constant_grammar::base_type( lit_const )
{
	lit_const %= tok.littok_int | tok.littok_float | tok.littok_bool;
}


#endif