#ifndef SASL_SYNTAX_TREE_SYNTAX_TREE_GENERATOR_H
#define SASL_SYNTAX_TREE_SYNTAX_TREE_GENERATOR_H

#include "program.h"
#include <string>

namespace sasl{
	namespace common{
		class lex_context;
	}
}

boost::shared_ptr<program> generate_syntax_tree( const string& code_text, lex_context* ctxt );

#endif