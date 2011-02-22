#include <sasl/include/parser/grammars.h>

#include <sasl/include/parser/lexer.h>

#define STERM( name ) terminal( lxr.get_id( #name ) )

BEGIN_NS_SASL_PARSER();

grammars::grammars( lexer& lxr ){
	prog = *decl;
	decl = 
		STERM(semicolon) 
		| ( 
			vardecl
		  ) >> STERM(semicolon);
	vardecl = STERM(ident) >> STERM(ident);
}

END_NS_SASL_PARSER();

