#ifndef SASL_PARSER_INSTANCING_H
#define SASL_PARSER_INSTANCING_H

#include <sasl/include/parser/parser_forward.h>
#include <sasl/include/parser/grammars.h>
#include <boost/preprocessor.hpp>

#define SASL_INSTANTIATE_GRAMMAR( grammar_name )										\
	void BOOST_PP_CAT(instanciate_, grammar_name)(){									\
		boost::shared_ptr<sasl::common::lex_context> ctxt;								\
		sasl_tokenizer tok(ctxt);																\
		sasl_grammar< sasl_token_iterator, sasl_skipper, sasl_tokenizer >* pg(NULL);	\
		grammar_name < sasl_token_iterator, sasl_skipper > instance( tok, *pg );		\
	}

#endif