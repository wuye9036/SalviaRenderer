#ifndef SASL_PARSER_DECLARATION_SPECIFIER_H
#define SASL_PARSER_DECLARATION_SPECIFIER_H

#include "parsers.h"
#include <boost/spirit/include/classic_ast.hpp>

template<typename ScannerT>
declaration_specifier::definition<ScannerT>::definition(const declaration_specifier& self){
	r_decl_spec = r_postfix_qualified_type;
	r_postfix_qualified_type =  r_prefix_qualified_type >> ( * r_postfix_type_qualifier );
	r_prefix_qualified_type = ( * r_prefix_type_qualifier ) >> r_unqualified_type;
	r_unqualified_type = 
		infix_node_d[ ch_p('(') >>  r_decl_spec >>  ')' ]
		| g_buildin_type 
		| g_struct_type
		| r_identifier_type
		;

	r_identifier_type = g_identifier;

	r_postfix_type_qualifier = 
		leaf_node_d[ r_keyword_qualifier ]
		| r_function_qualifier
		| r_array_qualifier
		;

	r_prefix_type_qualifier = 
		leaf_node_d[ r_keyword_qualifier ]
		;

	r_keyword_qualifier = 
		keyword_d[ str_p( "const" ) ]
		| keyword_d[ str_p( "volatile" ) ]
		| keyword_d[ str_p( "uniform" ) ]
		;

	r_function_qualifier = 
		infix_node_d[ ch_p('(') >> ! infix_node_d[( r_decl_spec >> *( ch_p(',') >> r_decl_spec ) )] >> ch_p(')') ];

	r_array_qualifier = 
		infix_node_d[ ch_p('[') >> ! g_expression >> ch_p(']') ];
}

template<typename ScannerT>
const RULE_TYPE(r_decl_spec)& declaration_specifier::definition<ScannerT>::start() const{
	return r_decl_spec;
}
#endif