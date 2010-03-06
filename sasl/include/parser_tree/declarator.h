#ifndef SASL_PARSER_TREE_DECLARATOR_H
#define SASL_PARSER_TREE_DECLARATOR_H

#include "parser_tree_forward.h"
#include "initializer.h"
#include <boost/fusion/adapted.hpp>
#include <boost/fusion/tuple.hpp>
#include <vector>

BEGIN_NS_SASL_PARSER_TREE()

struct initialized_declarator{
	typedef boost::optional<initializer> initializer_t;
	token_attr ident;
	initializer_t init;
};

struct initialized_declarator_list{
	typedef std::vector<
		boost::fusion::vector< token_attr, initialized_declarator >
	> follow_declarators_t;

	initialized_declarator first;
	follow_declarators_t follows;
};

END_NS_SASL_PARSER_TREE()

BOOST_FUSION_ADAPT_STRUCT( sasl::parser_tree::initialized_declarator,
						  (token_attr, ident)
						  (sasl::parser_tree::initialized_declarator::initializer_t, init)
						  );

BOOST_FUSION_ADAPT_STRUCT( sasl::parser_tree::initialized_declarator_list,
						  (sasl::parser_tree::initialized_declarator, first)
						  (sasl::parser_tree::initialized_declarator_list::follow_declarators_t, follows)
						  );

#endif