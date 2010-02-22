#ifndef SASL_PARSER_INSTANCING_H
#define SASL_PARSER_INSTANCING_H

#include "parser_forward.h"
#include "grammars.h"
#include <boost/preprocessor.hpp>

#define SASL_INSTANTIATE_GRAMMAR( grammar_name )										\
	void BOOST_PP_CAT(instanciate_, grammar_name)(){									\
		sasl_tokenizer tok;																\
		sasl_grammar< sasl_token_iterator, sasl_skipper, sasl_tokenizer >* pg(NULL);	\
		grammar_name < sasl_token_iterator, sasl_skipper > instance( tok, *pg );		\
	}

#endif