#ifndef SASL_PARSER_GRAMMARS_INITIALIZED_DECLARATOR_LIST_H
#define SASL_PARSER_GRAMMARS_INITIALIZED_DECLARATOR_LIST_H

#include "../parser_forward.h"
#include "../../parser_tree/declarator.h"
#include "../../syntax_tree/token.h"
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/lex.hpp>

SASL_DEFINE_GRAMMAR( initialized_declarator_list_grammar, sasl::parser_tree::initialized_declarator_list() ){
	template <typename TokenDefT, typename SASLGrammarT>
	initialized_declarator_list_grammar( const TokenDefT& tok, SASLGrammarT& g );

	SASL_GRAMMAR_RULE_DEFINITION_HELPER();

	typename rule<sasl::parser_tree::initialized_declarator_list()>::type start;
	typename rule<sasl::parser_tree::initialized_declarator()>::type init_declarator;
	typename rule<token_attr()>::type 
		ident, comma;
};

#endif