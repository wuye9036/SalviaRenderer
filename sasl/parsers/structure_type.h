#ifndef SASL_PARSER_STRUCTURE_TYPE_H
#define SASL_PARSER_STRUCTURE_TYPE_H

#include "parsers.h"
#include <boost/spirit/include/classic_ast.hpp>

template< typename ScannerT >
structure_type::definition< ScannerT >::definition(const structure_type& self)
{
	r_structure_type = 
		no_node_d[ keyword_p("struct") ]
		>> g_identifier
		>> "{"
		>> *g_declaration
		>> "}";
}

template< typename ScannerT >
const RULE_TYPE(r_structure_type)& structure_type::definition<ScannerT>::start() const{
	return r_structure_type;
}

#endif