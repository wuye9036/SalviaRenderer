#ifndef SASL_PARSER_INITIALIZED_DECLARATOR_H
#define SASL_PARSER_INITIALIZED_DECLARATOR_H

#include "parsers.h"
#include <boost/spirit/include/classic_ast.hpp>

template< typename ScannerT >
initialized_declarator::definition<ScannerT>::definition(const initialized_declarator& self)
{
	r_declarator = 
		g_identifier
		| inner_node_d[ ch_p('(') >> r_declarator >> ')' ]
		;

	r_initializer = 
		( ch_p('=') >> r_initialize_expr )
		| ( ch_p('(') >> g_expr_list >> ')' )
		;

	r_initialize_expr = 
		g_assign_expr
		| ch_p('{') >> '}'
		| ch_p('{') >> infix_node_d[ r_initialize_expr >> *( ch_p(',') >> r_initialize_expr ) ] >> '}'
		;

	r_initialized_declarator = r_declarator >> !r_initializer;
}

template< typename ScannerT >
const RULE_TYPE(r_initialized_declarator)& initialized_declarator::definition<ScannerT>::start() const{
	return r_initialized_declarator;
}

#endif