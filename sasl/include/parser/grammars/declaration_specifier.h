#ifndef SASL_PARSER_GRAMMARS_DECLARATION_SPECIFIER_H
#define SASL_PARSER_GRAMMARS_DECLARATION_SPECIFIER_H

#include <sasl/include/parser/parser_forward.h>

#include <sasl/include/common/token_attr.h>
#include <sasl/include/parser_tree/declaration_specifier.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/lex.hpp>
#include <eflib/include/platform/boost_end.h>

SASL_DEFINE_GRAMMAR( declaration_specifier_grammar, sasl::parser_tree::declaration_specifier() )
{
	template< typename TokenDefT, typename SASLGrammarT >
	declaration_specifier_grammar( const TokenDefT& tok, SASLGrammarT& g );

	SASL_GRAMMAR_RULE_DEFINITION_HELPER();

	typename rule<sasl::parser_tree::declaration_specifier()>::type start;

	typename rule<sasl::parser_tree::unqualified_type()>::type unqualed_type;
	typename rule<sasl::parser_tree::prefix_qualified_type()>::type prequaled_type;
	typename rule<sasl::parser_tree::postfix_qualified_type()>::type postqualed_type;

	typename rule<sasl::parser_tree::prefix_type_qualifier()>::type prefix_typequal;
	typename rule<sasl::parser_tree::postfix_type_qualifier()>::type postfix_typequal;

	typename rule<sasl::parser_tree::function_type_qualifier()>::type func_typequal;
	typename rule<sasl::parser_tree::array_type_qualifier()>::type array_typequal;

	typename rule<sasl::parser_tree::parameter_type_qualifier()>::type param_typequal;

	typename rule<sasl::common::token_attr()>::type 
		lparen, rparen, 
		lsbracket, rsbracket,
		keyword_typequal,
		ident
		;
};

#endif