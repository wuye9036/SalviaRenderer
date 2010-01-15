#include "../../include/parser/grammars/token.h"
#include "../../include/parser/detail/cast_expression.h"

void instantiate_cast_expression(){
	sasl_tokenizer tok;
	primary_expression_grammar<sasl_token_iterator, sasl_skipper>* pmexpr(NULL);
	expression_grammar<sasl_token_iterator, sasl_skipper>* expr(NULL);
	cast_expression_grammar<sasl_token_iterator, sasl_skipper> castexpr( tok, *pmexpr, *expr );
}