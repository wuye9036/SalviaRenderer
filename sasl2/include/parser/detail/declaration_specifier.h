#ifndef SASL_PARSER_DETAIL_DECLARATION_SPECIFIER_H
#define SASL_PARSER_DETAIL_DECLARATION_SPECIFIER_H

#include "../grammars/declaration_specifier.h"
#include "../grammars/expression.h"
#include "../grammars/declaration.h"
#include <boost/spirit/include/lex_lexertl.hpp>

template <typename IteratorT, typename LexerT>
template <typename TokenDefT, typename SASLGrammarT>
declaration_specifier_grammar<IteratorT, LexerT>::declaration_specifier_grammar( const TokenDefT& tok, SASLGrammarT& g )
: base_type( start )
{
	// init
	g.decl_spec(*this);

	// grammar
	struct_declaration_grammar<IteratorT, LexerT>& struct_decl = g.struct_decl();
	expression_grammar<IteratorT, LexerT>& expr = g.expr();

	// non-terminators
	start %= postqualed_type.alias();
	
	postqualed_type %= prequaled_type >> (* postfix_typequal );
	prequaled_type %= ( *prefix_typequal ) >> unqualed_type;

	unqualed_type %= 
		( lparen >> postqualed_type >> rparen )
		| struct_decl
		| ident
		;

	prefix_typequal %= keyword_typequal;
	postfix_typequal %= 
		keyword_typequal
		| func_typequal
		| array_typequal
		;

	func_typequal %= lparen >> (*param_typequal) >> rparen;
	array_typequal %= lsbracket > -expr > rsbracket;

	param_typequal %= start >> -ident;

	// terminators
	lparen %= tok.marktok_lparen;
	rparen %= tok.marktok_rparen;
	lsbracket %= tok.marktok_lsbracket;
	rsbracket %= tok.marktok_rsbracket;
	keyword_typequal %= tok.kwtok_uniform | tok.kwtok_shared;
	ident %= tok.littok_ident;

	// for debug
	start.name("declaration specifier");
	unqualed_type.name("unqualified type");
	prequaled_type.name("prefix-qualified type");
	postqualed_type.name("postfix-qualified type");
	prefix_typequal.name("prefix type qualifier");
	postfix_typequal.name("postfix type qualifier");
	func_typequal.name("function type qualifier");
	array_typequal.name("array type qualifier");
	param_typequal.name("parameter of function type");
	
	lparen.name("(");
	rparen.name(")");
	lsbracket.name("[");
	rsbracket.name("]");
	keyword_typequal.name("type_qualifier_keyword");
	ident.name("identifier");
}

#endif