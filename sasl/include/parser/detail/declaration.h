#ifndef SASL_PARSER_DETAIL_DECLARATION_H
#define SASL_PARSER_DETAIL_DECLARATION_H

#include <sasl/include/parser/grammars/declaration.h>
#include <sasl/include/parser/grammars/statement.h>
#include <sasl/include/parser/grammars/token.h>
#include <boost/spirit/include/lex_lexertl.hpp>

template<typename IteratorT, typename LexerT>
template<typename TokenDefT, typename SASLGrammarT>
declaration_grammar<IteratorT, LexerT>::declaration_grammar(const TokenDefT& tok, SASLGrammarT& g)
:base_type(decl)
{
	// init
	g.decl(*this);

	// grammar
	declaration_specifier_grammar< IteratorT, LexerT >& declspec = g.decl_spec();
	struct_declaration_grammar< IteratorT, LexerT >& struct_decl = g.struct_decl();
	statement_grammar< IteratorT, LexerT >& stmt = g.stmt();
	variable_declaration_grammar< IteratorT, LexerT>& vardecl = g.vardecl();

	// non-terminators
	decl %=
		basic_decl 
		| function_def;
	basic_decl %= 
		-( vardecl
		| function_decl
		| struct_decl
		| typedef_decl
		) >> semicolon
		;
	function_decl %= declspec >> ident >> ( lparen > (*param) > rparen );
	function_def %= function_decl >> function_body;
	
	typedef_decl %= kw_typedef > declspec > ident;

	param %= declspec >> -ident;
	function_body %= lbrace >> *stmt >> rbrace;

	// terminators
	semicolon %= tok.marktok_semicolon;
	lbrace %= tok.marktok_lbrace;
	rbrace %= tok.marktok_rbrace;
	kw_typedef %= tok.kwtok_typedef;
	lparen %= tok.marktok_lparen;
	rparen %= tok.marktok_rparen;
	ident %= tok.littok_ident;

	// for debug
	decl.name("declaration");
	basic_decl.name("basic declaration");
	function_def.name("function definition");
	function_decl.name("function declaration");
	function_body.name("function body");
	typedef_decl.name("type definition declaration");
	param.name("function parameter");
	
	semicolon.name(";");
	lbrace.name("{");
	rbrace.name("}");
	kw_typedef.name("typedef");
	lparen.name("(");
	rparen.name(")");
	ident.name("identifier");
}
#endif