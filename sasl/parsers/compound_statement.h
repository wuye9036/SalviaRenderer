#ifndef SASL_PARSER_COMPOUND_STATEMENT_H
#define SASL_PARSER_COMPOUND_STATEMENT_H

#include "parsers.h"

DEFINE_GRAMMAR_DEFINITION( compound_statement ){
	r_compound_statement = 
		infix_node_d[ ch_p('{') >> *g_stmt >> ch_p('}') ];
}

DEFINE_START_RULE( compound_statement, r_compound_statement )

#endif