#ifndef SASL_PARSEr_expr_list_H
#define SASL_PARSEr_expr_list_H

#include "parsers.h"
#include <boost/spirit/include/classic_ast.hpp>

template< typename ScannerT >
expression_list::definition<ScannerT>::definition(const expression_list &self)
{
	// can match one expression or more.
	// if you want a list semantic whatever, use this rule.
	r_expr_list = 
		g_assign_expr >> *( ch_p(',') >> g_assign_expr );
}

template< typename ScannerT >
const RULE_TYPE(r_expr_list)& expression_list::definition<ScannerT>::start() const{
	return r_expr_list;
}


#endif