#ifndef SASL_PARSER_GRAMMARS_INITIALIZER_H
#define SASL_PARSER_GRAMMARS_INITIALIZER_H

#include <sasl/include/parser/parser_forward.h>

#include <sasl/include/parser_tree/initializer.h>
#include <sasl/include/common/token_attr.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/lex.hpp>
#include <eflib/include/platform/boost_end.h>

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

	typename rule<sasl::common::token_attr()>::type 
		lbrace, rbrace, lparen, rparen,
		comma, equal;
};

#endif