#include "../../include/parser/grammars/token.h"
#include "../../include/parser/detail/expression.h"

void instantiate_expression(){
	sasl_tokenizer tok;
	expression_grammar<sasl_token_iterator, sasl_skipper> expr( tok );
}