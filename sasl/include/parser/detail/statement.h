#ifndef SASL_PARSER_DETAIL_STATEMENT_H
#define SASL_PARSER_DETAIL_STATEMENT_H

#include "../grammars/statement.h"
#include "../grammars/expression.h"
#include "../grammars/declaration.h"
#include "../grammars/token.h"
#include <boost/spirit/include/lex_lexertl.hpp>

template<typename IteratorT, typename LexerT>
template<typename TokenDefT, typename SASLGrammarT>
statement_grammar<IteratorT, LexerT>::statement_grammar( const TokenDefT& tok, SASLGrammarT& g )
:base_type(stmt){
	// init
	g.stmt(*this);

	// grammar
	expression_grammar<IteratorT, LexerT>& expr = g.expr();
	declaration_grammar<IteratorT, LexerT>& decl = g.decl();

	// non-terminators
	stmt %=
		stmt_decl
		| stmt_if
		| stmt_while
		| stmt_do_while
		| stmt_for
		| stmt_switch
		| stmt_expr
		| stmt_compound
		| stmt_flowctrl
		| labeled_stmt
		;
	stmt_if %=
		kw_if > 
		lparen > expr > rparen > 
		stmt > 
		-( kw_else > stmt )
		;
	stmt_for %= kw_for > for_looper > stmt;
	for_looper %= lparen > for_init_decl > -expr > semicolon > -expr > rparen;
	stmt_while %=
		kw_while > lparen > expr > rparen
		> stmt
		;
	stmt_do_while %=
		kw_do > stmt > kw_while > lparen > expr > rparen
		;
	stmt_expr %= expr >> semicolon;
	stmt_decl %= decl;
	stmt_compound %= lbrace > *stmt > rbrace ;
	stmt_flowctrl %=
		stmt_break
		| stmt_continue
		| stmt_return
		;
	stmt_switch %=
		kw_switch > lparen > expr > rparen
		> lbrace
		> *stmt
		> rbrace
		;
	stmt_break %= kw_break > semicolon;
	stmt_continue %= kw_continue > semicolon;
	stmt_return %= kw_return >> -expr >> semicolon;
	labeled_stmt %= 
		( ident | (kw_case > expr ) ) >> colon >> stmt;
	for_init_decl %= 
		stmt_decl 
		| stmt_expr
		;

	// terminators
	kw_break %= tok.kwtok_break;
	kw_continue %= tok.kwtok_continue;
	kw_case %= tok.kwtok_case;
	kw_return %= tok.kwtok_return;
	lbrace %= tok.marktok_lbrace;
	rbrace %= tok.marktok_rbrace;
	kw_switch %= tok.kwtok_switch;
	kw_else %= tok.kwtok_else;
	kw_for %= tok.kwtok_for;
	kw_if %= tok.kwtok_if;
	kw_while %= tok.kwtok_while;
	kw_do %= tok.kwtok_do;
	lparen %= tok.marktok_lparen;
	rparen %= tok.marktok_rparen;
	ident %= tok.littok_ident;
	colon %= tok.marktok_colon;
	semicolon %= tok.marktok_semicolon;

	// for debug
	stmt.name("statement");
	stmt_decl.name("declaration statement");
	stmt_if.name("if statement");
	stmt_while.name("while statement");
	stmt_do_while.name("do-while statement");
	stmt_for.name("for statement");
	stmt_switch.name("switch statement");
	stmt_expr.name("expression statement");
	stmt_compound.name("compound statement");
	stmt_flowctrl.name("flow-control statement");
	labeled_stmt.name("labeled statement");
	stmt_break.name("break statement");
	stmt_continue.name("continue statement");
	stmt_return.name("return statement");
	for_init_decl.name("for initialization declaration");

	kw_break.name("break");
	kw_continue.name("continue");
	kw_case.name("case");
	kw_return.name("return");
	lbrace.name("{");
	rbrace.name("}");
	kw_switch.name("switch");
	kw_else.name("else");
	kw_for.name("for");
	kw_if.name("if");
	kw_while.name("while");
	kw_do.name("do");
	lparen.name("(");
	rparen.name(")");
	ident.name("identifier");
	colon.name(":");
	semicolon.name(";");
};

#endif