#ifndef SASL_PARSER_GRAMMARS_STATEMENT_H
#define SASL_PARSER_GRAMMARS_STATEMENT_H

#include "../parser_forward.h"
#include "../../parser_tree/statement.h"
#include "../../syntax_tree/token.h"
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/lex.hpp>

SASL_DEFINE_GRAMMAR( statement_grammar, sasl::parser_tree::statement())
{
	template<typename TokenDefT, typename SASLGrammarT> statement_grammar( const TokenDefT&, SASLGrammarT& );

	SASL_GRAMMAR_RULE_DEFINITION_HELPER();

	typename rule<sasl::parser_tree::statement()>::type stmt;
	typename rule<sasl::parser_tree::if_statement()>::type stmt_if;
	typename rule<sasl::parser_tree::for_statement()>::type stmt_for;
	typename rule<sasl::parser_tree::for_loop_header()>::type for_looper;
	typename rule<sasl::parser_tree::while_statement()>::type stmt_while;
	typename rule<sasl::parser_tree::dowhile_statement()>::type stmt_do_while;
	typename rule<sasl::parser_tree::expression_statement()>::type stmt_expr;
	typename rule<sasl::parser_tree::declaration_statement()>::type stmt_decl;
	typename rule<sasl::parser_tree::compound_statement()>::type stmt_compound;
	typename rule<sasl::parser_tree::flowcontrol_statement()>::type stmt_flowctrl;
	typename rule<sasl::parser_tree::switch_statement()>::type stmt_switch;
	typename rule<sasl::parser_tree::jump_statement()>::type stmt_break, stmt_continue;
	typename rule<sasl::parser_tree::return_statement()>::type stmt_return;
	typename rule<sasl::parser_tree::labeled_statement()>::type labeled_stmt;
	typename rule<sasl::parser_tree::for_initializer()>::type for_init_decl;

	typename rule<token_attr()>::type
		kw_break,
		kw_continue,
		kw_case,
		kw_return,
		lbrace,
		rbrace,
		kw_switch,
		kw_else,
		kw_for,
		kw_if,
		kw_while,
		kw_do,
		lparen,
		rparen,
		semicolon,
		colon,
		ident
		;
};
#endif