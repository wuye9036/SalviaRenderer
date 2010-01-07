#include "../../include/parser/grammars/token.h"
#include "../../include/parser/detail/binary_expression.h"

void instantiate_binary_expression(){
	sasl_tokenizer tok;
	binary_expression_grammar<sasl_token_iterator, sasl_skipper> g( tok );
}