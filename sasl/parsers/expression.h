#ifndef SASL_PARSER_EXPRESSION_H
#define SASL_PARSER_EXPRESSION_H

#include "parsers.h"
#include <boost/spirit/include/classic_ast.hpp>

template< typename ScannerT >
expression::definition<ScannerT>::definition(const expression &self)
{
	// if one expression then
	//	return expr_list[0]
	// else 
	//	return expr_list as expr
	r_expression = g_expr_list;
}

template< typename ScannerT >
const RULE_TYPE(r_expression)& expression::definition<ScannerT>::start() const{
	return r_expression;
}

#endif