#include <sasl/include/syntax_tree/parse_api.h>

#include <sasl/include/parser/parse_api.h>
#include <sasl/include/parser/lexer.h>
#include <sasl/include/parser/grammars.h>
#include <sasl/include/syntax_tree/syntax_tree_builder.h>

using sasl::common::code_source;
using sasl::common::lex_context;
using sasl::common::diag_chat;
using sasl::parser::attribute;
using sasl::parser::lexer;
using sasl::parser::grammars;

using boost::shared_ptr;

BEGIN_NS_SASL_SYNTAX_TREE();

void init_lex( lexer& l ){
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
		("CARET", "\\^")
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
		( "kw_default", "default" )

		( "inc_dec", "{PLUS}{PLUS}|{MINUS}{MINUS}" )

		("add_assign",   "{PLUS}=")
		("sub_assign",   "{MINUS}=")
		("mul_assign",   "{ASTERISK}=")
		("div_assign",   "{SLASH}=")
		("mod_assign",   "%=")
		("band_assign",  "&=")
		("bor_assign",   "{VERTICAL}=")
		("bxor_assign",  "{CARET}=")
		("shift",		 "{LABRACKET}{LABRACKET}|{RABRACKET}{RABRACKET}")
		("shift_assign", "({LABRACKET}{LABRACKET}|{RABRACKET}{RABRACKET})=")

		( "less_equal", "{LABRACKET}=")
		( "greater_equal", "{RABRACKET}=")
		( "equal_to", "==" )
		( "not_equal", "!=" )

		( "logic_or", "{VERTICAL}{VERTICAL}" )
		( "logic_and", "&&" )

		( "plus", "{PLUS}" )
		( "minus", "{MINUS}" )
		( "asterisk", "{ASTERISK}" )
		( "slash", "{SLASH}")
		( "backslash", "{BACKSLASH}" )
		( "caret", "{CARET}" )
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

		( "space", "{SPACE}" )
		( "newline", "{NEWLINE}" )
		( "cppcomment", "{SLASH}{SLASH}[^\\n]*" )
		( "comment", "{SLASH}{ASTERISK}")

		( "anychar", "." )
		( "endcomment", "{ASTERISK}{SLASH}")
		;

	l.add_token( "INITIAL" )
		("lit_int")("lit_float")("lit_bool")
		("lparen")("rparen")("lbrace")("rbrace")
		
		("kw_struct")("kw_typedef")
		("kw_break")("kw_continue")("kw_case")("kw_default")("kw_return")
		("kw_switch")("kw_else")("kw_for")("kw_if")
		("kw_while")("kw_do")

		("ident")
		("shift_assign")("band_assign")("bor_assign")("bxor_assign")			// &= |= ^=
		("less_equal")("equal_to")("not_equal")("greater_equal")				// <= >= == !=
		("add_assign")("sub_assign")("mul_assign")("div_assign")("mod_assign")	// += -= *= /= %=
		("shift") // << >>
		("logic_or")("logic_and") // || &&
		("inc_dec")("exclamation")("tilde") // ++ -- ! ~
		("plus")("minus")("asterisk")("slash")("percent") // + - * / %
		("labracket")("rabracket") // < >
		("lsbracket")("rsbracket") // [ ]
		("vertical")("ampersand")("caret") // & | ^
		("question") // ?
		("comma")("colon")("semicolon") // , : ;
		("dot")("equal") // . =
		;

	l.add_token( "SKIPPED" )
		("space")
		("newline")
		("cppcomment")
		("comment", "COMMENT")
		;

	l.add_token( "COMMENT" )
		("endcomment", "INITIAL")
		("anychar")
		;

	l.skippers( "SKIPPED" )( "COMMENT" );
	l.init_states( "INITIAL" )( "SKIPPED" );
}

shared_ptr<program> parse(
	std::string const& code_text,
	shared_ptr<lex_context> ctxt,
	diag_chat* diags
	)
{
	lexer l;
	init_lex(l);
	grammars g(l);

	shared_ptr<sasl::parser::attribute> pt_prog;
	if( sasl::parser::parse( pt_prog, code_text, ctxt, l, g, diags ) )
	{
		syntax_tree_builder builder(l, g);
		return builder.build_prog( pt_prog );
	}
	return shared_ptr<program>();
}

shared_ptr<program> parse( code_source* src, shared_ptr<lex_context> ctxt, diag_chat* diags )
{
	lexer l;
	init_lex(l);
	grammars g(l);

	shared_ptr<sasl::parser::attribute> pt_prog;
	if( sasl::parser::parse( pt_prog, src, ctxt, l, g, diags ) )
	{
		syntax_tree_builder builder(l, g);
		return builder.build_prog( pt_prog );
	}
	return shared_ptr<program>();
}

END_NS_SASL_SYNTAX_TREE();

