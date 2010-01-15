#include "../../include/parser/grammars/token.h"
#include "../../include/parser/detail/literal_constant.h"

void instantiate_literal_constant(){
	sasl_tokenizer tok;
	literal_constant_grammar<sasl_token_iterator, sasl_skipper> litconst( tok );
}