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
		("PLUS", "\\+")
		("MINUS", "\\-")
		("ASTERISK", "\\*")
		("SLASH", "\\/")
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

	(littok_int = "[0-9]+");

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
	(marktok_colon = ";");
	(marktok_dot = ".");
	(marktok_exclamation = "!");
	(marktok_question = "QUESTION");
	(marktok_squote = "'");
	(marktok_dquote = "{DQUOTE}");
	(marktok_vertical = "{VERTICAL}");

	(marktok_lparen = "{LPAREN}");
	(marktok_rparen = "{RPAREN}");
	(marktok_rbrace = "{LBRACE}");
	(marktok_rbrace = "{RBRACE}");
	(marktok_left_square_bracket = "{LSBRACKET}");
	(marktok_right_square_bracket = "{RSBRACKET}");
	(marktok_left_angle_bracket = "{LABRACKET}");
	(marktok_right_angle_bracket = "{RABRACKET}");

	(optok_arith_assign = "\\*=|\\/=|%=|\\+=|\\-=|\\>\\>=|\\<\\<=|&=|\\^=|\\|=");
	(optok_shift = "\\<\\<|\\>\\>");
	(optok_equal = "==|\\!=");
	(optok_relation = "\\<=|\\>=");
	(optok_logic_and = "&&");
	(optok_logic_or = "\\|\\|");
	(optok_self_incr = "\\+\\+|\\-\\-");


	// composited operators


	(whitetok_space = "{SPACE}");
	(whitetok_newline = "{NEWLINE}");
	(whitetok_pp_line = "#line.*{NEWLINE}");
	(whitetok_c_comment = "\\/\\*[^*]*\\*+([^/*][^*]*\\*+)*\\/");
	(whitetok_cpp_comment = "\\/\\/.*{NEWLINE}");

	this->self =
		littok_int					[*attr_setter]
		| marktok_plus				[*attr_setter]
		| marktok_minus				[*attr_setter]
		| marktok_asterisk			[*attr_setter]
		| marktok_slash				[*attr_setter]
		| marktok_lparen			[*attr_setter]
		| marktok_rparen			[*attr_setter]
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