#include <sasl/include/parser/parse_api.h>

#include <sasl/include/common/lex_context.h>
#include <sasl/include/parser/lexer.h>
#include <sasl/include/parser/grammars.h>

#include <eflib/include/diagnostics/assert.h>

#include <iostream>

using sasl::common::lex_context;
using sasl::common::code_source;
using boost::shared_ptr;
using std::cout;
using std::endl;

void sasl::parser::parse( 
	shared_ptr<attribute>& pt_root,
	const std::string& code,
	shared_ptr<lex_context> ctxt,
	lexer& l, grammars& g
	)
{
	sasl::parser::token_seq toks;
	bool tok_result = l.tokenize(code, ctxt, toks);
	EFLIB_ASSERT( tok_result, "Tokenizing is failed." );	
	token_iterator it = toks.begin();
	g.prog.parse( it, toks.end(), pt_root );
}

void sasl::parser::parse( 
	shared_ptr<attribute>& pt_root,
	code_source* src,
	shared_ptr<lex_context > ctxt,
	lexer& l, grammars& g
	)
{
	sasl::parser::token_seq toks;

	l.begin_incremental();
	while( !src->is_eof() ){
		bool tok_result = l.incremental_tokenize( src->next_token(), ctxt, toks );
		EFLIB_ASSERT_AND_IF( tok_result, "Tokenizing is failed." ){	break; }
	}
	l.end_incremental();

	token_iterator it = toks.begin();
	g.prog.parse( it, toks.end(), pt_root );
}
