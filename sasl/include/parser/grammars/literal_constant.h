#ifndef SASL_PARSER_GRAMMARS_LITERAL_CONSTANT_H
#define SASL_PARSER_GRAMMARS_LITERAL_CONSTANT_H

#include <sasl/include/parser/parser_forward.h>

#include <sasl/include/common/token_attr.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/lex.hpp>
#include <eflib/include/platform/boost_end.h>

SASL_DEFINE_GRAMMAR( literal_constant_grammar, sasl::common::token_attr() ){
	template <typename TokenDefT, typename SASLGrammarT>
	literal_constant_grammar( const TokenDefT& tok, SASLGrammarT& g );

	SASL_GRAMMAR_RULE_DEFINITION_HELPER();

	typename rule<sasl::common::token_attr()>::type lit_const;
};

#endif