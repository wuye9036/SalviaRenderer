#ifndef SASL_PARSER_DETAIL_DECLARATION_SPECIFIER_H
#define SASL_PARSER_DETAIL_DECLARATION_SPECIFIER_H

#include <sasl/include/parser/grammars/declaration_specifier.h>

#include <sasl/include/parser/detail/grammar_impl_base.h>
#include <sasl/include/parser/grammars/expression.h>
#include <sasl/include/parser/grammars/declaration.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/spirit/include/lex_lexertl.hpp>
#include <eflib/include/platform/boost_end.h>

template <typename IteratorT, typename LexerT>
struct declspec_impl: public grammar_impl_base_t{

	SASL_GRAMMAR_RULE_DEFINITION_HELPER();

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

template <typename IteratorT, typename LexerT>
template <typename TokenDefT, typename SASLGrammarT>
declaration_specifier_grammar<IteratorT, LexerT>::declaration_specifier_grammar( const TokenDefT& tok, SASLGrammarT& g )
: base_type( start )
{
	// init
	g.decl_spec(*this);

	pimpl.reset( new declspec_impl<IteratorT, LexerT>() );
	declspec_impl<IteratorT, LexerT>& impl = * ( boost::shared_polymorphic_cast< declspec_impl<IteratorT, LexerT> >(pimpl) );

	// grammar
	struct_declaration_grammar<IteratorT, LexerT>& struct_decl = g.struct_decl();
	expression_grammar<IteratorT, LexerT>& expr = g.expr();


	// non-terminators
	start %= impl.postqualed_type.alias();
	
	impl.postqualed_type %= impl.prequaled_type >> (* impl.postfix_typequal );
	impl.prequaled_type %= ( *impl.prefix_typequal ) >> impl.unqualed_type;

	impl.unqualed_type %= 
		( impl.lparen >> impl.postqualed_type >> impl.rparen )
		| struct_decl
		| impl.ident
		;

	impl.prefix_typequal %= impl.keyword_typequal;
	impl.postfix_typequal %= 
		impl.keyword_typequal
		| impl.func_typequal
		| impl.array_typequal
		;

	impl.func_typequal %= impl.lparen >> (*impl.param_typequal) >> impl.rparen;
	impl.array_typequal %= impl.lsbracket > -expr > impl.rsbracket;

	impl.param_typequal %= start >> -impl.ident;

	// terminators
	impl.lparen %= tok.marktok_lparen;
	impl.rparen %= tok.marktok_rparen;
	impl.lsbracket %= tok.marktok_lsbracket;
	impl.rsbracket %= tok.marktok_rsbracket;
	impl.keyword_typequal %= tok.kwtok_uniform | tok.kwtok_shared;
	impl.ident %= tok.littok_ident;

	// for debug
	start.name("declaration specifier");
	impl.unqualed_type.name("unqualified type");
	impl.prequaled_type.name("prefix-qualified type");
	impl.postqualed_type.name("postfix-qualified type");
	impl.prefix_typequal.name("prefix type qualifier");
	impl.postfix_typequal.name("postfix type qualifier");
	impl.func_typequal.name("function type qualifier");
	impl.array_typequal.name("array type qualifier");
	impl.param_typequal.name("parameter of function type");
	
	impl.lparen.name("(");
	impl.rparen.name(")");
	impl.lsbracket.name("[");
	impl.rsbracket.name("]");
	impl.keyword_typequal.name("type_qualifier_keyword");
	impl.ident.name("identifier");
}

#endif