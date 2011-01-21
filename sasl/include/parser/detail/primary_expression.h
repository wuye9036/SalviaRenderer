#ifndef SASL_PARSER_DETAIL_PRIMARY_EXPRESSION_H
#define SASL_PARSER_DETAIL_PRIMARY_EXPRESSION_H

#include <sasl/include/parser/detail/grammar_impl_base.h>

#include <sasl/include/parser/grammars/expression.h>
#include <sasl/include/parser/grammars/literal_constant.h>
#include <sasl/include/parser/grammars/token.h>

#include <boost/spirit/include/lex_lexertl.hpp>

template <typename IteratorT, typename LexerT>
struct grammar_impl: public grammar_impl_base_t{
	
	SASL_GRAMMAR_RULE_DEFINITION_HELPER();

	typename rule<sasl::parser_tree::paren_expression()>::type	parenexpr;
	typename rule<sasl::common::token_attr()>::type				ident;
	typename rule<sasl::common::token_attr()>::type				lparen, rparen;

};

template <typename IteratorT, typename LexerT>
template <typename TokenDefT, typename SASLGrammarT>
primary_expression_grammar<IteratorT, LexerT>::primary_expression_grammar(
	const TokenDefT& tok, SASLGrammarT& g )
: base_type( pmexpr, "primary expression grammar" )
{
	// init
	g.primary_expr(*this);

	grammar_impl<IteratorT, LexerT>& impl = *( static_cast< grammar_impl<IteratorT, LexerT>* >(pimpl.get()) );

	// grammar
	literal_constant_grammar<IteratorT, LexerT>& lit_const = g.lit_const();
	expression_grammar<IteratorT, LexerT>& expr = g.expr();

	// non terminators
	pmexpr %= lit_const | impl.ident | impl.parenexpr;
	impl.parenexpr %=  impl.lparen > expr > impl.rparen;

	// terminators
	impl.ident %= tok.littok_ident;
	impl.lparen %= tok.marktok_lparen;
	impl.rparen %= tok.marktok_rparen;

	// for debug
	pmexpr.name("primary expression");
	impl.parenexpr.name("paren expression");
	
	impl.ident.name("identifier");
	impl.lparen.name("(");
	impl.rparen.name(")");
}

#endif