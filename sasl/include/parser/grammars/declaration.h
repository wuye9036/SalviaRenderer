#ifndef SASL_PARSER_GRAMMARS_VARIABLE_DECLARATION_H
#define SASL_PARSER_GRAMMARS_VARIABLE_DECLARATION_H

#include <sasl/include/parser/parser_forward.h>

#include <sasl/include/common/token_attr.h>
#include <sasl/include/parser_tree/declaration.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/spirit/include/lex.hpp>
#include <boost/spirit/include/qi.hpp>
#include <eflib/include/platform/boost_end.h>

SASL_DEFINE_GRAMMAR( variable_declaration_grammar, sasl::parser_tree::variable_declaration() )
{
	template<typename TokenDefT, typename SASLGrammarT>
	variable_declaration_grammar(const TokenDefT& tok, SASLGrammarT& g);

	SASL_GRAMMAR_RULE_DEFINITION_HELPER();

	typename rule<sasl::parser_tree::variable_declaration()>::type vardecl;
	typename rule<sasl::common::token_attr()>::type
		semicolon
		;
};

SASL_DEFINE_GRAMMAR( declaration_grammar, sasl::parser_tree::declaration() ){
	template<typename TokenDefT, typename SASLGrammarT>
	declaration_grammar(const TokenDefT& tok, SASLGrammarT& g);

	SASL_GRAMMAR_RULE_DEFINITION_HELPER();

	typename rule<sasl::parser_tree::declaration()>::type decl;
	typename rule<sasl::parser_tree::basic_declaration()>::type basic_decl;
	typename rule<sasl::parser_tree::function_declaration()>::type function_decl;
	typename rule<sasl::parser_tree::function_definition()>::type function_def;
	typename rule<sasl::parser_tree::function_body()>::type function_body;
	typename rule<sasl::parser_tree::typedef_declaration()>::type typedef_decl;
	typename rule<sasl::parser_tree::function_parameter()>::type param;
	
	typename rule<sasl::common::token_attr()>::type 
		semicolon, lbrace, rbrace, kw_typedef, lparen, rparen, ident
		;
};

SASL_DEFINE_GRAMMAR( struct_declaration_grammar, sasl::parser_tree::struct_declaration() ){
	template< typename TokenDefT, typename SASLGrammarT >
	struct_declaration_grammar( const TokenDefT& tok, SASLGrammarT& decl);

	SASL_GRAMMAR_RULE_DEFINITION_HELPER();

	typename rule<sasl::parser_tree::struct_declaration()>::type struct_decl;
	typename rule<sasl::parser_tree::nonamed_struct_body()>::type struct_body;
	typename rule<sasl::parser_tree::named_struct_body()>::type named_struct_body;
	typename rule<sasl::common::token_attr()>::type
		kw_struct, ident, lbrace, rbrace
		;
};
#endif