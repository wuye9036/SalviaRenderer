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
	shared_ptr< ::sasl::common::lex_context > ctxt
	)
{
	sasl::parser::token_seq toks;
	
	lexer l;

	l.add_pattern
		("SPACE", "[ \\t\\v\\f]+")
		("NEWLINE", "((\\r\\n?)|\\n)+")
		("NON_ZERO_DIGIT", "[1-9]")
		("DIGIT", "[0-9]")
		("SIGN", "[\\+\\-]")
		("DIGIT_SEQ", "([0-9]+)")
		("HEX_DIGIT", "[0-9a-fA-F]")
		("EXPONENT_PART", "((e|E)[+-]?[0-9]+)")
		("REAL_TYPE_SUFFIX", "[fFdD]")
		("INT_TYPE_SUFFIX", "([uULl]|([Uu][Ll)|([Ll][Uu]))")
		("PLUS", "\\+")
		("MINUS", "\\-")
		("ASTERISK", "\\*")
		("SLASH", "\\/")
		("DOT", "\\.")
		("BACKSLASH", "\\\\")
		("AMPERSAND", "\\^")
		("QUESTION", "\\?")
		("DQUOTE", "\\\"")
		("VERTICAL", "\\|")
		("LPAREN", "\\(")
		("RPAREN", "\\)")
		("LBRACE", "\\{")
		("RBRACE", "\\}")
		("LSBRACKET", "\\[")
		("RSBRACKET", "\\]")
		("LABRACKET", "\\<")
		("RABRACKET", "\\>")
		;

	l.define_tokens
		( "lit_int", "({DIGIT}+{INT_TYPE_SUFFIX}?)|(0x{HEX_DIGIT}+{INT_TYPE_SUFFIX}?)" )
		( "lit_float", "({DIGIT_SEQ}?{DOT}{DIGIT_SEQ}{EXPONENT_PART}?{REAL_TYPE_SUFFIX}?)|({DIGIT_SEQ}{EXPONENT_PART}{REAL_TYPE_SUFFIX}?)|({DIGIT_SEQ}{REAL_TYPE_SUFFIX})" )
		( "lit_bool", "(true)|(false)" )
		( "ident", "[a-zA-Z_][0-9a-zA-Z_]*")

		( "kw_struct", "struct" )
		( "kw_typedef", "typedef" )
		( "kw_break", "break" )
		( "kw_continue", "continue" )
		( "kw_case", "case" )
		( "kw_return", "return" )
		( "kw_switch", "switch" )
		( "kw_else", "else" )
		( "kw_for", "for" )
		( "kw_if", "if" )
		( "kw_while", "while" )
		( "kw_do", "do" )

		( "plus", "{PLUS}" )
		( "minus", "{MINUS}" )
		( "asterisk", "{ASTERISK}" )
		( "slash", "{SLASH}")
		( "backslash", "{BACKSLASH}" )
		( "caret", "{AMPERSAND}" )
		( "ampersand", "&" )
		( "percent", "%" )
		( "equal", "=" )
		( "comma", "," )
		( "colon", ":" )
		( "semicolon", ";" )
		( "dot", "{DOT}" )
		( "exclamation", "!" )
		( "question", "{QUESTION}" )
		( "squote", "'" )
		( "dquote", "{DQUOTE}" )
		( "vertical", "{VERTICAL}" )
		( "tilde", "~" )

		( "lparen", "{LPAREN}" )
		( "rparen", "{RPAREN}" )
		( "lbrace", "{LBRACE}" )
		( "rbrace", "{RBRACE}" )
		( "lsbracket", "{LSBRACKET}" )
		( "rsbracket", "{RSBRACKET}" )
		( "labracket", "{LABRACKET}" )
		( "rabracket", "{RABRACKET}" )

		( "any_char", "." )

		( "space", "{SPACE}" )
		( "newline", "{NEWLINE}" )
		;

	l.add_token( "INITIAL" )
		("ident")("semicolon")
		;

	l.add_token( "SKIPPED" )
		("space")("newline")
		;

	l.skippers( "SKIPPED" );

	l.tokenize(code, ctxt, toks );

	grammars g(l);
	
	token_iterator it = toks.begin();
	g.prog.parse( it, toks.end(), pt_root );
}