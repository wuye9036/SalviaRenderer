#include <sasl/include/parser/parse_api.h>

#include <sasl/include/common/lex_context.h>
#include <sasl/include/parser/lexer.h>
#include <sasl/include/parser/grammars.h>

#include <iostream>

using boost::shared_ptr;
using std::cout;
using std::endl;

void sasl::parser::parse( 
	shared_ptr<attribute>& pt_root,
	const std::string& code,
	shared_ptr< ::sasl::common::lex_context > ctxt,
	lexer& l, grammars& g
	)
{
	sasl::parser::token_seq toks;
	l.tokenize(code, ctxt, toks );
	token_iterator it = toks.begin();
	g.prog.parse( it, toks.end(), pt_root );
}