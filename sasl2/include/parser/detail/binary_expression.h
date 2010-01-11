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
	expr %= binexpr.alias();

	binexpr %= 
		pmexpr >> *( literal_op >  pmexpr );

	pmexpr %= 
		literal_int
		| parenexpr;

	parenexpr %=  literal_lparen > expr > literal_rparen;

	literal_op %= tok.marktok_plus | tok.marktok_minus;
	literal_lparen %= tok.marktok_lparen;
	literal_rparen %= tok.marktok_rparen;

	literal_int %= tok.littok_int;

	expr.name("expr");
	binexpr.name("binexpr");
	pmexpr.name("pmexpr");
	parenexpr.name("parenexpr");

	literal_op.name("literal_op");
	literal_lparen.name("literal_lparen");
	literal_rparen.name("literal_rparen");
	literal_int.name("literal_int");
}

#endif // SASL_PARSER_BINARY_EXPRESSION_H