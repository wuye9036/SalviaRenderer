#ifndef SASL_PARSER_TREE_INITIALIZER_H
#define SASL_PARSER_TREE_INITIALIZER_H

#include "parser_tree_forward.h"
#include "expression.h"
#include <boost/variant.hpp>
#include <boost/fusion/adapted.hpp>
#include <boost/fusion/tuple.hpp>
#include <vector>
BEGIN_NS_SASL_PARSER_TREE();

struct paren_initializer;
struct nullable_initializer_list;

typedef boost::variant< 
	boost::recursive_wrapper<assign_expression>,
	boost::recursive_wrapper<nullable_initializer_list>
> c_style_initializer;

struct initializer_list{
	typedef std::vector<
		boost::fusion::vector< token_attr, c_style_initializer >
	> inits_t;

	c_style_initializer first;
	inits_t follows;
};

typedef boost::fusion::vector<token_attr, c_style_initializer> initializer;

struct paren_initializer{
	typedef boost::optional<expression_lst> exprs_t;
	token_attr lparen;
	exprs_t exprs;
	token_attr rparen;
};

struct nullable_initializer_list{
	typedef boost::optional<initializer_list> inits_t;
	token_attr lbrace; 
	inits_t inits;
	token_attr rbrace;
};

END_NS_SASL_PARSER_TREE()

BOOST_FUSION_ADAPT_STRUCT( sasl::parser_tree::initializer_list,
						  (sasl::parser_tree::c_style_initializer, first)
						  (sasl::parser_tree::initializer_list::inits_t, follows)
						  );

BOOST_FUSION_ADAPT_STRUCT( sasl::parser_tree::paren_initializer,
						  (token_attr, lparen)
						  (sasl::parser_tree::paren_initializer::exprs_t, exprs)
						  (token_attr, rparen)
						  );

BOOST_FUSION_ADAPT_STRUCT( sasl::parser_tree::nullable_initializer_list,
						  (token_attr, lbrace)
						  (sasl::parser_tree::nullable_initializer_list::inits_t, inits)
						  (token_attr, rbrace)
						  );
#endif