#ifndef SASL_PARSER_DECLARATION_H
#define SASL_PARSER_DECLARATION_H

#include "parsers.h"
#include <boost/spirit/include/classic_ast.hpp>

template< typename ScannerT >
declaration::definition<ScannerT>::definition(const declaration& self){
	r_declaration = 
		(
		r_block_declaration 
		| r_function_definition
		| r_struct_definition
		)
		;
	
	r_block_declaration = 
		g_decl_spec
		>> g_init_decl_list
		>> no_node_d[str_p(";")]
	;

	r_function_definition = 
		g_decl_spec >> g_identifier >> r_parameters 
		>> ( g_compound_statement | no_node_d[ch_p(';') ] );

	r_parameter_item = 
		g_decl_spec 
		>> !( g_identifier >> !( no_node_d[ch_p('=')] >> g_assign_expr ) )
		;

	r_parameters = 
		no_node_d[ch_p('(')]
		>> ! ( r_parameter_item >> *( no_node_d[ch_p(',')] >> r_parameter_item ) )
		>> no_node_d[ch_p( ')' )]
		;

	r_struct_definition = g_struct_type;
}

template< typename ScannerT >
const RULE_TYPE(r_declaration)& declaration::definition<ScannerT>::start() const{
	return r_declaration;
}

#endif