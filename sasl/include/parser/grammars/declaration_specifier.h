#ifndef SASL_PARSER_GRAMMARS_DECLARATION_SPECIFIER_H
#define SASL_PARSER_GRAMMARS_DECLARATION_SPECIFIER_H

#include <sasl/include/parser/parser_forward.h>

#include <sasl/include/common/token_attr.h>
#include <sasl/include/parser_tree/declaration_specifier.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/lex.hpp>
#include <eflib/include/platform/boost_end.h>

struct grammar_impl_base_t;

SASL_DEFINE_GRAMMAR( declaration_specifier_grammar, sasl::parser_tree::declaration_specifier() )
{
	template< typename TokenDefT, typename SASLGrammarT >
	declaration_specifier_grammar( const TokenDefT& tok, SASLGrammarT& g );

	SASL_GRAMMAR_RULE_DEFINITION_HELPER();

	boost::shared_ptr< grammar_impl_base_t > pimpl;

	typename rule<sasl::parser_tree::declaration_specifier()>::type start;
};

#endif