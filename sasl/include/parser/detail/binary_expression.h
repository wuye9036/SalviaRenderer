#ifndef SASL_PARSER_DETAIL_BINARY_EXPRESSION_H
#define SASL_PARSER_DETAIL_BINARY_EXPRESSION_H

#include <sasl/include/parser/grammars/expression.h>
#include <sasl/include/parser/grammars/token.h>
#include <boost/spirit/include/lex_lexertl.hpp>

template <typename IteratorT, typename LexerT>
template <typename TokenDefT, typename SASLGrammarT>
binary_expression_grammar<IteratorT, LexerT>::binary_expression_grammar(
	const TokenDefT& tok, SASLGrammarT& g )
: base_type( lorexpr )
{	
	// init
	g.bin_expr(*this);

	// grammar
	cast_expression_grammar<IteratorT, LexerT>& castexpr = g.cast_expr();

	// non-terminators
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

	// terminators
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

	// for debug
	lorexpr.name("or expression");
	landexpr.name("and expression");
	borexpr.name("bit-or expression");
	bxorexpr.name("bit-exclusive-or expression");
	bandexpr.name("bit-and expression");
	eqlexpr.name("equal expression");
	relexpr.name("relation expression");
	shfexpr.name("shift expression");
	addexpr.name("add expression");
	mulexpr.name("mul expression");

	opadd.name("+/-");
	opmul.name("*///%");
	oprel.name("<>");
	opshift.name("<</>>");
	opequal.name("==");
	opband.name("&");
	opbxor.name("^");
	opbor.name("|");
	opland.name("&&");
	oplor.name("||");

}

#endif // SASL_PARSER_BINARY_EXPRESSION_H