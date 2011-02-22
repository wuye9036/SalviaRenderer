#ifndef SASL_PARSER_GRAMMARS_H
#define SASL_PARSER_GRAMMARS_H

#include <sasl/include/parser/generator.h>
#include <sasl/include/parser/parser_forward.h>

BEGIN_NS_SASL_PARSER();

class lexer;
class grammars{
public:
	grammars( lexer& lxr );
	rule prog, decl, vardecl;
};

END_NS_SASL_PARSER();

#endif