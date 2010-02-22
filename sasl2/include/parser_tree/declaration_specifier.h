#ifndef SASL_PARSER_TREE_DECLARATION_SPECIFIER_H
#define SASL_PARSER_TREE_DECLARATION_SPECIFIER_H

#include "parser_tree_forward.h"
#include "expression.h"
#include <boost/fusion/adapted.hpp>
#include <boost/variant.hpp>
#include <vector>

BEGIN_NS_SASL_PARSER_TREE()

typedef token_attr keyword_type_qualifier;
typedef keyword_type_qualifier prefix_type_qualifier;

struct function_type_qualifier;
struct array_type_qualifier;
struct prefix_qualified_type;
struct postfix_qualified_type;
struct struct_declaration;

typedef boost::variant<
	boost::recursive_wrapper< keyword_type_qualifier >,
	boost::recursive_wrapper< function_type_qualifier >,
	boost::recursive_wrapper< array_type_qualifier >
> postfix_type_qualifier;

typedef boost::fusion::vector<token_attr, postfix_qualified_type, token_attr> paren_post_qualified_type;

typedef boost::variant<
	boost::recursive_wrapper<token_attr>,
	boost::recursive_wrapper<paren_post_qualified_type>,
	boost::recursive_wrapper<struct_declaration>
> unqualified_type;

struct prefix_qualified_type{
	typedef std::vector<prefix_type_qualifier> qualifiers_t;
	qualifiers_t quals;
	unqualified_type unqual_type;
};

struct postfix_qualified_type{
	typedef std::vector<postfix_type_qualifier> qualifiers_t;
	prefix_qualified_type unqual_type;
	qualifiers_t quals;
};

typedef postfix_qualified_type declaration_specifier;

struct parameter_type_qualifier{
	typedef boost::optional<token_attr> ident_t;
	declaration_specifier declspec;
	ident_t ident;
};

struct function_type_qualifier{
	typedef std::vector<parameter_type_qualifier> param_quals_t;
	token_attr lparen;
	param_quals_t params;
	token_attr rparen;
};

struct array_type_qualifier{
	typedef boost::optional<expression> array_size_expr_t;
	token_attr lsbracket;
	array_size_expr_t size_expr;
	token_attr rsbracket;
};

END_NS_SASL_PARSER_TREE()

BOOST_FUSION_ADAPT_STRUCT( sasl::parser_tree::prefix_qualified_type,
						  ( sasl::parser_tree::prefix_qualified_type::qualifiers_t, quals )
						  ( sasl::parser_tree::unqualified_type, unqual_type )
						  );

BOOST_FUSION_ADAPT_STRUCT( sasl::parser_tree::postfix_qualified_type,
						  (sasl::parser_tree::prefix_qualified_type, unqual_type)
						  (sasl::parser_tree::postfix_qualified_type::qualifiers_t, quals)
						  );

BOOST_FUSION_ADAPT_STRUCT( sasl::parser_tree::parameter_type_qualifier,
						  (sasl::parser_tree::declaration_specifier, declspec)
						  (sasl::parser_tree::parameter_type_qualifier::ident_t, ident)
						  );

BOOST_FUSION_ADAPT_STRUCT( sasl::parser_tree::function_type_qualifier,
						  (token_attr, lparen)
						  (sasl::parser_tree::function_type_qualifier::param_quals_t, params)
						  (token_attr, rparen)
						  );

BOOST_FUSION_ADAPT_STRUCT( sasl::parser_tree::array_type_qualifier,
						  (token_attr, lsbracket)
						  (sasl::parser_tree::array_type_qualifier::array_size_expr_t, size_expr)
						  (token_attr, rsbracket)
						  );
#endif