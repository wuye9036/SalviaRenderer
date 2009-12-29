#ifndef SASL_PARSER_TREE_LITERAL_H
#define SASL_PARSER_TREE_LITERAL_H

#include "parser_tree_forward.h"
#include "../syntax_tree/token.h"

BEGIN_NS_SASL_PARSER_TREE()

typedef token_attr constant;
typedef token_attr operator_literal;

END_NS_SASL_PARSER_TREE()

#endif