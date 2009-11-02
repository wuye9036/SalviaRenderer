#ifndef SASL_PARSER_IDENTIFIER_H
#define SASL_PARSER_IDENTIFIER_H

#include "parsers.h"

DEFINE_GRAMMAR_DEFINITION( identifier ){
	r_identifier = leaf_node_d[ lexeme_d[( alpha_p | '_' ) >> * ( alnum_p )] ];
}

DEFINE_START_RULE( identifier, r_identifier );

#endif