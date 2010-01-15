#include "../../include/parser/grammars/token.h"
#include "../../include/parser/detail/primary_expression.h"

void instantiate_primary_expression(){
	sasl_tokenizer tok;
	expression_grammar<sasl_token_iterator, sasl_skipper>* expr(NULL);
	primary_expression_grammar<sasl_token_iterator, sasl_skipper> pmexpr( tok, *expr );
}