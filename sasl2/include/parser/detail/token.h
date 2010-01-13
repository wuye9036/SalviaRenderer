#ifndef SASL_PARSER_DETAIL_TOKEN_H
#define SASL_PARSER_DETAIL_TOKEN_H

#include "../grammars/token.h"
#include "../../parser_tree/literal.h"
#include "../../enums/token_types.h"
#include <string>

struct lex_context{
	lex_context(): line(0), column(0) {}
	size_t line;
	size_t column;
};

struct token_attribute_setter{
	token_attribute_setter( lex_context& ctxt ): lex_ctxt( ctxt ){
	}
	token_attribute_setter( const token_attribute_setter& rhs ): lex_ctxt( rhs.lex_ctxt ){
	}

	template <typename IteratorT, typename PassFlagT, typename IdtypeT, typename ContextT>
	void operator () (IteratorT& start, IteratorT& end, PassFlagT& matched, IdtypeT& id, ContextT& ctx){
		token_attr attr;
		attr.lit.assign(start, end);
		
		attr.column = lex_ctxt.column;
		attr.file_name = "undefined";
		attr.line = lex_ctxt.line;

		ctx.set_value( attr );

		lex_ctxt.column += attr.lit.length();
	}

	lex_context& lex_ctxt;
};

template <typename BaseLexerT>
sasl_tokens<BaseLexerT>::sasl_tokens(){
	ctxt.reset( new lex_context() );
	attr_setter.reset(new token_attribute_setter(*ctxt));

	this->self.add_pattern
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

	// literals
	(littok_int = "({DIGIT}+{INT_TYPE_SUFFIX}?)|(0x{HEX_DIGIT}+{INT_TYPE_SUFFIX}?)");
	(littok_float = "({DIGIT_SEQ}?{DOT}{DIGIT_SEQ}{EXPONENT_PART}?{REAL_TYPE_SUFFIX}?)|({DIGIT_SEQ}{EXPONENT_PART}{REAL_TYPE_SUFFIX}?)|({DIGIT_SEQ}{REAL_TYPE_SUFFIX})");
	(littok_bool = "(true)|(false)");

	(littok_ident = "[a-zA-Z_][0-9a-zA-Z_]*");

	// markers
	(marktok_plus = "{PLUS}");
	(marktok_minus = "{MINUS}");
	(marktok_asterisk = "{ASTERISK}");
	(marktok_slash = "{SLASH}");
	(marktok_backslash = "{BACKSLASH}");
	(marktok_caret = "{AMPERSAND}");
	(marktok_ampersand = "&");
	(marktok_percent = "%");
	(marktok_equal = "=");
	(marktok_comma = ",");
	(marktok_colon = ":");
	(marktok_semicolon = ";");
	(marktok_dot = "{DOT}");
	(marktok_exclamation = "!");
	(marktok_question = "{QUESTION}");
	(marktok_squote = "'");
	(marktok_dquote = "{DQUOTE}");
	(marktok_vertical = "{VERTICAL}");
	(marktok_tilde = "~");

	(marktok_lparen = "{LPAREN}");
	(marktok_rparen = "{RPAREN}");
	(marktok_lbrace = "{LBRACE}");
	(marktok_rbrace = "{RBRACE}");
	(marktok_lsbracket = "{LSBRACKET}");
	(marktok_rsbracket = "{RSBRACKET}");
	(marktok_labracket = "{LABRACKET}");
	(marktok_rabracket = "{RABRACKET}");

	// composited operators
	(optok_arith_assign = "\\*=|\\/=|%=|\\+=|\\-=|\\>\\>=|\\<\\<=|&=|\\^=|\\|=");
	(optok_shift = "\\<\\<|\\>\\>");
	(optok_equal = "==|\\!=");
	(optok_relation = "\\<=|\\>=");
	(optok_logic_and = "&&");
	(optok_logic_or = "\\|\\|");
	(optok_self_incr = "\\+\\+|\\-\\-");

	(whitetok_space = "{SPACE}");
	(whitetok_newline = "{NEWLINE}");
	(whitetok_pp_line = "#line.*{NEWLINE}");
	(whitetok_c_comment = "\\/\\*[^*]*\\*+([^/*][^*]*\\*+)*\\/");
	(whitetok_cpp_comment = "\\/\\/.*{NEWLINE}");

	this->self =
		// literal
		littok_int					[*attr_setter]
		| littok_float				[*attr_setter]
		| littok_bool				[*attr_setter]
		// operator
		| optok_arith_assign		[*attr_setter]
		| optok_shift				[*attr_setter]
		| optok_equal				[*attr_setter]
		| optok_relation			[*attr_setter]
		| optok_logic_and			[*attr_setter]
		| optok_logic_or			[*attr_setter]
		| optok_self_incr			[*attr_setter]
		// mark
		| marktok_plus				[*attr_setter]
		| marktok_minus				[*attr_setter]
		| marktok_asterisk			[*attr_setter]
		| marktok_slash				[*attr_setter]
		| marktok_backslash			[*attr_setter]
		| marktok_caret				[*attr_setter]
		| marktok_ampersand			[*attr_setter]
		| marktok_percent			[*attr_setter]
		| marktok_equal				[*attr_setter]
		| marktok_comma				[*attr_setter]
		| marktok_colon				[*attr_setter]
		| marktok_semicolon			[*attr_setter]
		| marktok_dot				[*attr_setter]
		| marktok_exclamation		[*attr_setter]
		| marktok_question			[*attr_setter]
		| marktok_squote			[*attr_setter]
		| marktok_dquote			[*attr_setter]
		| marktok_vertical			[*attr_setter]
		| marktok_tilde				[*attr_setter]

		| marktok_lparen			[*attr_setter]
		| marktok_rparen			[*attr_setter]
		| marktok_lbrace			[*attr_setter]
		| marktok_rbrace			[*attr_setter]
		| marktok_lsbracket			[*attr_setter]
		| marktok_rsbracket			[*attr_setter]
		| marktok_labracket			[*attr_setter]
		| marktok_rabracket			[*attr_setter]

		// keywords

		// identifier
		| littok_ident				[*attr_setter]
		;

	this->self("SKIPPED") =
		whitetok_space				[*attr_setter]
		| whitetok_newline			[*attr_setter]
		| whitetok_pp_line			[*attr_setter]
		| whitetok_c_comment		[*attr_setter]
		| whitetok_cpp_comment		[*attr_setter]							
		;
}



#endif //SASL_PARSER_TOKEN_H