#ifndef SASL_PARSER_DETAIL_BINARY_EXPRESSION_H
#define SASL_PARSER_DETAIL_BINARY_EXPRESSION_H

#include "../grammars/expression.h"
#include "../../parser_tree/expression.h"
#include <boost/spirit/include/lex_lexertl.hpp>

#include <iostream>
#include <string>
struct lazy_print{
	lazy_print(const std::string& str): str(str){
	}

	template <typename Attrib, typename Context>
	void operator()(Attrib const&, Context&, bool&) const{
		std::cout << str << std::endl;
	}

	std::string str;
};

template <typename IteratorT, typename LexerT>
template <typename TokenDefT>
binary_expression_grammar<IteratorT, LexerT>::binary_expression_grammar( const TokenDefT& tok ): binary_expression_grammar::base_type( expr )
{
	expr %= exprlst.alias();

	exprlst %= assignexpr >> *( comma > assignexpr );
	assignexpr %= rhsexpr >> *( opassign > rhsexpr );
	rhsexpr %= condexpr | lorexpr;

	condexpr %= lorexpr >> ( question > expr > colon > assignexpr );
	
	lorexpr %= landexpr >> *( oplor > landexpr );
	landexpr %= borexpr >> *( opland > borexpr );
	
	borexpr %= bxorexpr >> *( opbor > bxorexpr );
	bxorexpr %= bandexpr >> *( opbxor > bandexpr );
	bandexpr %= eqlexpr >> *( opband > eqlexpr );

	eqlexpr %= relexpr >> *( opequal > relexpr );
	relexpr %= shfexpr >> *( oprel > shfexpr );

	shfexpr %= addexpr >> *( opshift > addexpr );

	addexpr %= mulexpr >> *( opadd > mulexpr );
	mulexpr %= castexpr >> *( opmul >  castexpr );

	castexpr %= unaryexpr | typecastedexpr;
	typecastedexpr %= lparen > identifier > rparen > expr;

	unaryexpr %= postexpr | unariedexpr;
	unariedexpr = (opinc | opunary) > castexpr;

	postexpr %= 
		pmexpr >> *( idxexpr | callexpr | memexpr | opinc )
		;

	pmexpr %= 
		literal_int
		| parenexpr;

	idxexpr %= lsbracket > expr > rsbracket;
	callexpr %=	lparen > -expr > rparen;
	memexpr %= opmember > identifier;

	parenexpr %=  lparen > expr > rparen;

	opadd %= tok.marktok_plus | tok.marktok_minus;
	opmul %= tok.marktok_asterisk | tok.marktok_slash | tok.marktok_percent;
	opinc %= tok.optok_self_incr;
	opmember %= tok.marktok_dot;
	oprel %= tok.optok_relation | tok.marktok_labracket | tok.marktok_rabracket;
	opshift %= tok.optok_shift;
	opequal %= tok.optok_equal;
	opband %= tok.marktok_ampersand;
	opbxor %= tok.marktok_caret;
	opbor %= tok.marktok_vertical;
	opland %= tok.optok_logic_and;
	oplor %= tok.optok_logic_or;
	opassign %= tok.marktok_equal | tok.optok_arith_assign;
	opunary %= tok.marktok_plus | tok.marktok_minus | tok.marktok_tilde | tok.marktok_exclamation;
	lparen %= tok.marktok_lparen;
	rparen %= tok.marktok_rparen;
	lsbracket %= tok.marktok_lsbracket;
	rsbracket %= tok.marktok_rsbracket;
	comma %= tok.marktok_comma;
	question %= tok.marktok_question;
	colon %= tok.marktok_colon;

	literal_int %= tok.littok_int;
	literal_ident %= tok.littok_ident;
	literal_bool %= tok.littok_bool;
	literal_float %= tok.littok_float;

	expr.name("expr");
	addexpr.name("addexpr");
	mulexpr.name("mulexpr");
	pmexpr.name("pmexpr");
	parenexpr.name("parenexpr");

	lparen.name("lparen");
	rparen.name("rparen");
	literal_int.name("literal_int");
}

#endif // SASL_PARSER_BINARY_EXPRESSION_H