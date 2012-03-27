#include <sasl/include/parser/parse_api.h>

#include <sasl/include/common/lex_context.h>
#include <sasl/include/parser/lexer.h>
#include <sasl/include/parser/grammars.h>
#include <sasl/include/parser/diags.h>
#include <sasl/include/common/diag_chat.h>

#include <eflib/include/diagnostics/assert.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/format.hpp>
#include <eflib/include/platform/boost_end.h>

#include <iostream>

namespace sasl{
	namespace common{
		class diag_chat;
	}
}
using sasl::common::lex_context;
using sasl::common::code_source;
using sasl::common::diag_chat;
using boost::shared_ptr;
using std::cout;
using std::endl;

bool sasl::parser::parse( 
	shared_ptr<attribute>& pt_root,
	const std::string& code,
	shared_ptr<lex_context> ctxt,
	lexer& l, grammars& g, diag_chat* diags
	)
{
	sasl::parser::token_seq toks;
	
	bool tok_result = l.tokenize_with_end(code, ctxt, toks);
	if(!tok_result)
	{
		diags->report( sasl::parser::unrecognized_token )
			->file( ctxt->file_name() )->span( sasl::common::code_span(ctxt->line(), ctxt->column(), 1) )
			->p("<unknown>");
		return false;
	}

	token_iterator it = toks.begin();
	return g.prog.parse( it, toks.end()-1, pt_root, diags ).is_succeed();
}

bool sasl::parser::parse( 
	shared_ptr<attribute>& pt_root,
	code_source* src,
	shared_ptr<lex_context > ctxt,
	lexer& l, grammars& g, diag_chat* diags
	)
{
	sasl::parser::token_seq toks;

	l.begin_incremental();
	while( !src->eof() ){
		std::string next_token = src->next();
		bool tok_result = l.incremental_tokenize( next_token, ctxt, toks );
		if( !tok_result ){
			diags->report( sasl::parser::unrecognized_token )
				->file( ctxt->file_name() )->span( sasl::common::code_span(ctxt->line(), ctxt->column(), 1) )
				->p(next_token);
			return false;
		}
	}
	l.end_incremental( ctxt, toks );

	token_iterator it = toks.begin();
	return g.prog.parse( it, toks.end()-1, pt_root, diags ).is_succeed();
}
