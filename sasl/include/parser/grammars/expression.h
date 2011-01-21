#ifndef SASL_PARSER_GRAMMARS_EXPRESSION_H
#define SASL_PARSER_GRAMMARS_EXPRESSION_H

#include <sasl/include/parser/parser_forward.h>

#include <sasl/include/common/token_attr.h>

#include <sasl/include/parser_tree/expression.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/lex.hpp>
#include <eflib/include/platform/boost_end.h>

struct grammar_impl_base_t;

SASL_DEFINE_GRAMMAR( primary_expression_grammar, sasl::parser_tree::pm_expression() ){
	template <typename TokenDefT, typename SASLGrammarT>
	primary_expression_grammar( const TokenDefT& tok, SASLGrammarT& g );

	SASL_GRAMMAR_RULE_DEFINITION_HELPER();

	boost::scoped_ptr< grammar_impl_base_t > pimpl;

	typename rule<sasl::parser_tree::pm_expression()>::type		pmexpr;
};

SASL_DEFINE_GRAMMAR( cast_expression_grammar, sasl::parser_tree::cast_expression()){
	template<typename TokenDefT, typename SASLGrammarT>
	cast_expression_grammar(const TokenDefT& tok, SASLGrammarT& g);

	SASL_GRAMMAR_RULE_DEFINITION_HELPER();

	// boost::scoped_ptr< grammar_impl_base_t > pimpl;

	typename rule<sasl::parser_tree::cast_expression()>::type		castexpr;
	typename rule<sasl::parser_tree::typecast_expression()>::type	typecastedexpr;
	typename rule<sasl::parser_tree::unary_expression()>::type		unaryexpr;
	typename rule<sasl::parser_tree::unaried_expression()>::type	unariedexpr;
	typename rule<sasl::parser_tree::post_expression()>::type		postexpr;
	typename rule<sasl::parser_tree::expression_lst()>::type		exprlst;

	typename rule<sasl::parser_tree::idx_expression()>::type		idxexpr;
	typename rule<sasl::parser_tree::call_expression()>::type		callexpr;
	typename rule<sasl::parser_tree::mem_expression()>::type		memexpr;

	typename rule<sasl::common::token_attr()>::type
		ident,
		opinc,
		opmember,
		opunary,
		lparen, rparen,
		lsbracket, rsbracket
		;
};

SASL_DEFINE_GRAMMAR( binary_expression_grammar, sasl::parser_tree::lor_expression() ){
	template <typename TokenDefT, typename SASLGrammarT>
	binary_expression_grammar(const TokenDefT& tok, SASLGrammarT& g);

	SASL_GRAMMAR_RULE_DEFINITION_HELPER();

	// boost::scoped_ptr< grammar_impl_base_t > pimpl;

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

	typename rule<sasl::common::token_attr()>::type
		opadd,
		opmul,
		oprel,
		opshift,
		opband, opbxor, opbor,
		opland, oplor,
		opequal,
		ident
		;
};

SASL_DEFINE_GRAMMAR( expression_grammar, sasl::parser_tree::expression() ){
	template <typename TokenDefT, typename SASLGrammarT>
	expression_grammar( const TokenDefT& tok, SASLGrammarT& g );

	SASL_GRAMMAR_RULE_DEFINITION_HELPER();

	// boost::scoped_ptr< grammar_impl_base_t > pimpl;

	typename rule<sasl::parser_tree::expression()>::type expr;

	typename rule<sasl::parser_tree::expression_lst()>::type exprlst;
	typename rule<sasl::parser_tree::assign_expression()>::type	assignexpr;
	typename rule<sasl::parser_tree::rhs_expression()>::type	rhsexpr;
	typename rule<sasl::parser_tree::cond_expression()>::type	condexpr;

	typename rule<sasl::common::token_attr()>::type
		opassign,
		comma, question, colon
		;
};

SASL_DEFINE_GRAMMAR( assignment_expression_grammar, sasl::parser_tree::assign_expression() ){
	
	template <typename TokenDefT, typename SASLGrammarT>
	assignment_expression_grammar(const TokenDefT& tok, SASLGrammarT& g);

	SASL_GRAMMAR_RULE_DEFINITION_HELPER();

	// boost::scoped_ptr< grammar_impl_base_t > pimpl;

	typename rule<sasl::parser_tree::assign_expression()>::type	assignexpr;
	typename rule<sasl::parser_tree::rhs_expression()>::type	rhsexpr;
	typename rule<sasl::parser_tree::cond_expression()>::type	condexpr;

	typename rule<sasl::common::token_attr()>::type
		opassign,
		question, colon
		;
};

SASL_DEFINE_GRAMMAR( expression_list_grammar, sasl::parser_tree::expression_lst() ){
	template <typename TokenDefT, typename SASLGrammarT>
	expression_list_grammar(const TokenDefT& tok, SASLGrammarT& g);
	
	SASL_GRAMMAR_RULE_DEFINITION_HELPER();

	// boost::scoped_ptr< grammar_impl_base_t > pimpl;

	typename rule<sasl::parser_tree::expression_lst()>::type exprlst;

	typename rule<sasl::common::token_attr()>::type comma;
};

#endif