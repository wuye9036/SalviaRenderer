#ifndef SASL_PARSER_GRAMMARS_EXPRESSION_H
#define SASL_PARSER_GRAMMARS_EXPRESSION_H

#include "../parser_forward.h"
#include "literal_constant.h"
#include "../../parser_tree/expression.h"
#include "../../parser_tree/literal.h"
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/lex.hpp>

template <typename IteratorT, typename LexerT> struct expression_grammar;

DEFINE_GRAMMAR( primary_expression_grammar, sasl::parser_tree::pm_expression() ){
	template <typename TokenDefT>
	primary_expression_grammar(
		const TokenDefT& tok,
		expression_grammar<IteratorT, LexerT>& e );

	expression_grammar<IteratorT, LexerT>& expr;
	
	RULE_DEFINE_HELPER();

	typename rule<sasl::parser_tree::pm_expression()>::type			pmexpr;
	typename rule<sasl::parser_tree::paren_expression()>::type		parenexpr;
	typename rule<sasl::parser_tree::identifier_literal()>::type	ident;
	typename rule<sasl::parser_tree::operator_literal()>::type		lparen, rparen;
	literal_constant_grammar<IteratorT, LexerT> lit_const;
};

DEFINE_GRAMMAR( cast_expression_grammar, sasl::parser_tree::cast_expression()){
	template<typename TokenDefT>
	cast_expression_grammar(
		const TokenDefT& tok,
		primary_expression_grammar<IteratorT, LexerT>& pmexpr_,
		expression_grammar<IteratorT, LexerT>& expr_);

	RULE_DEFINE_HELPER();

	typename rule<sasl::parser_tree::cast_expression()>::type		castexpr;
	typename rule<sasl::parser_tree::typecast_expression()>::type	typecastedexpr;
	typename rule<sasl::parser_tree::unary_expression()>::type		unaryexpr;
	typename rule<sasl::parser_tree::unaried_expression()>::type	unariedexpr;
	typename rule<sasl::parser_tree::post_expression()>::type		postexpr;
	typename rule<sasl::parser_tree::expression_lst()>::type		exprlst;

	typename rule<sasl::parser_tree::idx_expression()>::type		idxexpr;
	typename rule<sasl::parser_tree::call_expression()>::type		callexpr;
	typename rule<sasl::parser_tree::mem_expression()>::type		memexpr;
	
	primary_expression_grammar<IteratorT, LexerT>& pmexpr;
	expression_grammar<IteratorT, LexerT>& expr;

	typename rule<sasl::parser_tree::identifier_literal()>::type	ident;
	typename rule<sasl::parser_tree::operator_literal()>::type
		opinc,
		opmember,
		opunary,
		lparen, rparen,
		lsbracket, rsbracket
		;
};

DEFINE_GRAMMAR( binary_expression_grammar, sasl::parser_tree::lor_expression() ){
	template <typename TokenDefT>
	binary_expression_grammar(
		const TokenDefT& tok,
		cast_expression_grammar<IteratorT, LexerT>& castexpr_ );

	RULE_DEFINE_HELPER();

	cast_expression_grammar<IteratorT, LexerT>& castexpr;

	typename rule<sasl::parser_tree::lor_expression()>::type	lorexpr;
	typename rule<sasl::parser_tree::land_expression()>::type	landexpr;
	typename rule<sasl::parser_tree::bor_expression()>::type	borexpr;
	typename rule<sasl::parser_tree::bxor_expression()>::type	bxorexpr;
	typename rule<sasl::parser_tree::band_expression()>::type	bandexpr;
	typename rule<sasl::parser_tree::eql_expression()>::type	eqlexpr;
	typename rule<sasl::parser_tree::rel_expression()>::type	relexpr;
	typename rule<sasl::parser_tree::shf_expression()>::type	shfexpr;
	typename rule<sasl::parser_tree::add_expression()>::type	addexpr;
	typename rule<sasl::parser_tree::mul_expression()>::type	mulexpr;
	typename rule<sasl::parser_tree::operator_literal()>::type
		opadd,
		opmul,
		oprel,
		opshift,
		opband, opbxor, opbor,
		opland, oplor,
		opequal
		;

	typename rule<sasl::parser_tree::identifier_literal()>::type
		identifier;
};

DEFINE_GRAMMAR( expression_grammar, sasl::parser_tree::expression() ){
	template <typename TokenDefT>
	expression_grammar( const TokenDefT& tok );

	RULE_DEFINE_HELPER();

	typename rule<sasl::parser_tree::expression()>::type expr;

	typename rule<sasl::parser_tree::expression_lst()>::type exprlst;
	typename rule<sasl::parser_tree::assign_expression()>::type	assignexpr;
	typename rule<sasl::parser_tree::rhs_expression()>::type	rhsexpr;
	typename rule<sasl::parser_tree::cond_expression()>::type	condexpr;

	typename rule<sasl::parser_tree::operator_literal()>::type
		opassign,
		comma, question, colon
		;

	boost::shared_ptr<binary_expression_grammar<IteratorT, LexerT> > plorexpr;
	boost::shared_ptr<cast_expression_grammar<IteratorT, LexerT> > pcastexpr;
	boost::shared_ptr<primary_expression_grammar<IteratorT, LexerT> > ppmexpr;
};


#endif