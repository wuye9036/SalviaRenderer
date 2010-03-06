#ifndef SASL_PARSER_DETAIL_STRUCT_DECLARATION_H
#define SASL_PARSER_DETAIL_STRUCT_DECLARATION_H

#include "../grammars/declaration.h"
#include <boost/spirit/include/lex_lexertl.hpp>

template<typename IteratorT, typename LexerT>
template<typename TokenDefT, typename SASLGrammarT>
struct_declaration_grammar<IteratorT, LexerT>::struct_declaration_grammar(const TokenDefT& tok, SASLGrammarT& g)
: base_type( struct_decl ){
	// init
	g.struct_decl(*this);
	
	// grammar
	declaration_grammar< IteratorT, LexerT >& decl = g.decl();

	// non-terminators
	struct_decl %= kw_struct > ( struct_body | named_struct_body );
	named_struct_body %= ident >> -struct_body;
	struct_body %= lbrace >> *decl >> rbrace;

	// terminators
	kw_struct %= tok.kwtok_struct;
	ident %= tok.littok_ident;
	lbrace %= tok.marktok_lbrace;
	rbrace %= tok.marktok_rbrace;

	// for debug
	struct_decl.name("struct declaration");
	named_struct_body.name("named struct body");
	struct_body.name("anonymous struct body");
	
	kw_struct.name("struct");
	ident.name("identifier");
	lbrace.name("{");
	rbrace.name("}");
}

#endif