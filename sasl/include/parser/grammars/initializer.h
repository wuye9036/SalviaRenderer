#ifndef SASL_PARSER_GRAMMARS_INITIALIZER_H
#define SASL_PARSER_GRAMMARS_INITIALIZER_H

#include "../parser_forward.h"
#include "../parser_tree/initializer.h"
#include "../syntax_tree/token.h"
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/lex.hpp>

SASL_DEFINE_GRAMMAR( initializer_grammar, sasl::parser_tree::initializer() )
{
	template<typename TokenDefT, typename SASLGrammarT>
	initializer_grammar(const TokenDefT& tok, SASLGrammarT& g);

	SASL_GRAMMAR_RULE_DEFINITION_HELPER();

	typename rule<sasl::parser_tree::initializer()>::type start;
	typename rule<sasl::parser_tree::paren_initializer()>::type paren_initializer;
	typename rule<sasl::parser_tree::c_style_initializer()>::type c_style_initializer;
	typename rule<sasl::parser_tree::nullable_initializer_list()>::type nullable_initlst;
	typename rule<sasl::parser_tree::initializer_list()>::type initializer_list;

	typename rule<token_attr()>::type 
		lbrace, rbrace, lparen, rparen,
		comma, equal;
};

#endif