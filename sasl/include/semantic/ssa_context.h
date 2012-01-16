#ifndef SASL_SEMANTIC_SSA_CONTEXT_H
#define SASL_SEMANTIC_SSA_CONTEXT_H

#include <sasl/include/semantic/ssa_context.h>

BEGIN_NS_SASL_SEMANTIC();

struct expr_t;
struct block_t;
struct variable_t;
struct value_t;

class ssa_context
{
	value_t*	create_value();
	variable_t*	create_variable();
	block_t*	create_block();
	expr_t*		create_expr();
};

END_NS_SASL_SEMANTIC();

#endif