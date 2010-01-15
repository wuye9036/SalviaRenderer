#ifndef SASL_PARSER_DETAIL_BINARY_EXPRESSION_H
#define SASL_PARSER_DETAIL_BINARY_EXPRESSION_H

#include "../grammars/expression.h"
#include "../../parser_tree/expression.h"
#include <boost/spirit/include/lex_lexertl.hpp>

template <typename IteratorT, typename LexerT>
template <typename TokenDefT>
binary_expression_grammar<IteratorT, LexerT>::binary_expression_grammar(
	const TokenDefT& tok, 
	cast_expression_grammar<IteratorT, LexerT>& castexpr_ )
: binary_expression_grammar::base_type( lorexpr ), castexpr(castexpr_)
{	
	lorexpr %= landexpr >> *( oplor > landexpr );
	landexpr %= borexpr >> *( opland > borexpr );
	
	borexpr %= bxorexpr >> *( opbor > bxorexpr );
	bxorexpr %= bandexpr >> *( opbxor > bandexpr );
	bandexpr %= eqlexpr >> *( opband > eqlexpr );

	eqlexpr %= relexpr >> *( opequal > relexpr );
	relexpr %= shfexpr >> *( oprel > shfexpr );

	shfexpr %= addexpr >> *( opshift > addexpr );

	addexpr %= mulexpr >> *( opadd > mulexpr );
	mulexpr %= castexpr >> *( opmul > castexpr );

	
	opadd %= tok.marktok_plus | tok.marktok_minus;
	opmul %= tok.marktok_asterisk | tok.marktok_slash | tok.marktok_percent;
	
	oprel %= tok.optok_relation | tok.marktok_labracket | tok.marktok_rabracket;
	opshift %= tok.optok_shift;
	opequal %= tok.optok_equal;
	opband %= tok.marktok_ampersand;
	opbxor %= tok.marktok_caret;
	opbor %= tok.marktok_vertical;
	opland %= tok.optok_logic_and;
	oplor %= tok.optok_logic_or;
}

#endif // SASL_PARSER_BINARY_EXPRESSION_H