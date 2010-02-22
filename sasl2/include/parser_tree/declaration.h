#ifndef SASL_PARSER_TREE_DECLARATION_H
#define SASL_PARSER_TREE_DECLARATION_H

#include "parser_tree_forward.h"
#include "declaration_specifier.h"
#include "declarator.h"
#include "statement.h"
#include <boost/optional.hpp>
#include <boost/variant.hpp>

BEGIN_NS_SASL_PARSER_TREE()

struct basic_declaration;
struct variable_declaration;
struct function_declaration;
struct function_definition;
struct struct_declaration;
struct function_parameter;
struct function_body;
struct named_struct_body;
struct nonamed_struct_body;
struct typedef_declaration;

struct variable_declaration{
	declaration_specifier declspec;
	initialized_declarator_list decllist;
	// token_attr semicolon;
};
typedef boost::variant< 
	boost::recursive_wrapper< function_definition >, 
	boost::recursive_wrapper< basic_declaration >
> declaration;
struct basic_declaration{
	typedef boost::optional<
		boost::variant<
			boost::recursive_wrapper< variable_declaration >,
			boost::recursive_wrapper< function_declaration >,
			boost::recursive_wrapper< struct_declaration >,
			boost::recursive_wrapper< typedef_declaration >
			> 
		> decl_body_t;
	decl_body_t decl_body;
	token_attr semicolon;
};

struct function_declaration{
	typedef boost::fusion::vector< token_attr, std::vector< function_parameter >, token_attr > parameters_t;
	declaration_specifier return_type;
	token_attr ident;
	parameters_t params;
};
struct function_body{
	typedef std::vector<statement> statements_t;
	token_attr lbrace;
	statements_t stmt;
	token_attr rbrace;
};
struct function_definition{
	typedef std::vector<statement> statements_t;
	function_declaration decl;
	function_body body;
};
struct struct_declaration{
	typedef boost::variant<
		boost::recursive_wrapper< nonamed_struct_body >,
		boost::recursive_wrapper< named_struct_body >
	> body_t;
	token_attr struct_keyword;
	body_t body;
};

struct typedef_declaration{
	token_attr typedef_keyword;
	declaration_specifier def;
	token_attr ident;
	// token_attr semicolon;
};

struct nonamed_struct_body{
	typedef std::vector<declaration> declarations_t;
	token_attr lbrace;
	declarations_t decls;
	token_attr rbrace;
};

struct named_struct_body{
	typedef boost::optional<nonamed_struct_body> body_t;
	token_attr ident;
	body_t body;
};

struct function_parameter{
	typedef boost::optional<token_attr> param_name_t;
	declaration_specifier declspec;
	param_name_t name;
};


END_NS_SASL_PARSER_TREE()

BOOST_FUSION_ADAPT_STRUCT( sasl::parser_tree::basic_declaration,
						  ( sasl::parser_tree::basic_declaration::decl_body_t, decl_body )
						  ( token_attr, semicolon )
						  );

BOOST_FUSION_ADAPT_STRUCT( sasl::parser_tree::function_declaration,
						  ( sasl::parser_tree::declaration_specifier, return_type )
						  ( sasl::parser_tree::identifier_literal, ident )
						  ( sasl::parser_tree::function_declaration::parameters_t, params )
						  );

BOOST_FUSION_ADAPT_STRUCT( sasl::parser_tree::function_definition,
						  ( sasl::parser_tree::function_declaration, decl )
						  ( sasl::parser_tree::function_body, body )
						  );

BOOST_FUSION_ADAPT_STRUCT( sasl::parser_tree::struct_declaration,
						  ( token_attr, struct_keyword )
						  ( sasl::parser_tree::struct_declaration::body_t, body )
						  );

BOOST_FUSION_ADAPT_STRUCT( sasl::parser_tree::typedef_declaration,
						  ( token_attr, typedef_keyword )
						  ( sasl::parser_tree::declaration_specifier, def )
						  ( token_attr, ident )
						  // ( token_attr, semicolon )
						  );

BOOST_FUSION_ADAPT_STRUCT( sasl::parser_tree::nonamed_struct_body,
						  ( token_attr, lbrace )
						  ( sasl::parser_tree::nonamed_struct_body::declarations_t, decls )
						  ( token_attr, rbrace )
						  );

BOOST_FUSION_ADAPT_STRUCT( sasl::parser_tree::named_struct_body,
						  ( token_attr, ident )
						  ( sasl::parser_tree::named_struct_body::body_t, body )
						  );

BOOST_FUSION_ADAPT_STRUCT( sasl::parser_tree::function_parameter,
						  ( sasl::parser_tree::declaration_specifier, declspec )
						  ( sasl::parser_tree::function_parameter::param_name_t, name )
						  );

BOOST_FUSION_ADAPT_STRUCT( sasl::parser_tree::function_body,
						  ( token_attr, lbrace )
						  ( sasl::parser_tree::function_body::statements_t, stmt )
						  ( token_attr, rbrace )
						  );

BOOST_FUSION_ADAPT_STRUCT( sasl::parser_tree::variable_declaration,
						  (sasl::parser_tree::declaration_specifier, declspec)
						  (sasl::parser_tree::initialized_declarator_list, decllist)
						  // (token_attr, semicolon)
						  );
#endif