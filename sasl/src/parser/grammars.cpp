#include <sasl/include/parser/grammars.h>

#include <sasl/include/parser/error_handlers.h>
#include <sasl/include/parser/lexer.h>

#define STERM( name ) terminal( lxr.get_id( #name ), #name )
#define STERM2( name, desc ) terminal( lxr.get_id( #name ), desc )

#define SRULE( rule_name, def ) \
	rule_name = def;	\
	rule_name.name( #rule_name );

BEGIN_NS_SASL_PARSER();

grammars::grammars( lexer& lxr ):lxr(lxr){
	set_terms();
	set_prog();
	set_decls();
	set_exprs();
	set_typespecs();
	set_inits();
	set_stmts();
}

void grammars::set_prog(){
	SRULE( prog, *decl > eof );
}

void grammars::set_decls()
{
	SRULE( decl, function_def | ( function_decl > semicolon ) | basic_decl );
	SRULE(
		basic_decl,
		semicolon | 
		(
			( struct_decl
			| typedef_decl
			| vardecl 
			) > semicolon
		)
		);
	SRULE( function_def, function_decl >> function_body	);
	SRULE( vardecl, declspec >> decllist );
	SRULE( function_decl, declspec >> ident >> ( lparen >> -( param >> *( comma > param ) ) > rparen ) >> -sem );
	SRULE( struct_decl, kw_struct > ( struct_body | named_struct_body ) );
	SRULE( typedef_decl, kw_typedef > declspec > ident );
	SRULE( param, declspec >> -ident >> -sem >> -init );
	SRULE( function_body, stmt_compound );
	SRULE( decllist, init_declarator >> *(comma > init_declarator) );
	SRULE( init_declarator, ident >> -sem >> -anno >> -init );
	SRULE( sem, ( colon > ident ) >> -( lparen > lit_int > rparen ) );
	SRULE( anno, labracket >> *(ident > ident > equal > expr > semicolon ) > rsbracket );
	SRULE( named_struct_body, ident >> -struct_body );
	SRULE( struct_body, lbrace >> *decl > rbrace );
}

void grammars::set_exprs()
{
	SRULE( expr, exprlst );
	SRULE( exprlst, assignexpr >> *( comma > assignexpr ) );
	SRULE( assignexpr, rhsexpr >> *(opassign > rhsexpr) );
	SRULE( rhsexpr, condexpr | lorexpr );
	SRULE( condexpr, lorexpr >> (question > expr > colon > assignexpr ) );
	SRULE( lorexpr, landexpr >> *(oplor > landexpr) );
	SRULE( landexpr, borexpr >> *(opland > borexpr) );
	SRULE( borexpr, bxorexpr >> *(opbor > bxorexpr) );
	SRULE( bxorexpr, bandexpr >> *(opbxor > bandexpr) );
	SRULE( bandexpr, eqlexpr >> *(opband > eqlexpr) );
	SRULE( eqlexpr, relexpr >> *(opequal > relexpr) );
	SRULE( relexpr, shfexpr >> *(oprel > shfexpr) );
	SRULE( shfexpr, addexpr >> *( opshift > addexpr ) );
	SRULE( addexpr, mulexpr >> *( opadd > mulexpr ) );
	SRULE( mulexpr, castexpr >> *( opmul > castexpr ) );
	SRULE( castexpr, unaryexpr | typecastedexpr );
	SRULE( typecastedexpr, ( lparen > declspec > rparen ) > expr );
	SRULE( unaryexpr, postexpr | unariedexpr );
	SRULE( unariedexpr, (opinc | opunary) > castexpr );
	SRULE( postexpr, pmexpr >> *(idxexpr | callexpr | memexpr | opinc) );
	SRULE( idxexpr, lsbracket > expr > rsbracket );
	SRULE( callexpr, lparen >> -exprlst > rparen );
	SRULE( memexpr, opmember > ident );
	SRULE( pmexpr, lit_const | ident | parenexpr );
	SRULE( parenexpr, lparen > expr > rparen );
}

void grammars::set_typespecs()
{
	SRULE( declspec, postqualed_type );
	SRULE( postqualed_type, prequaled_type >> *postfix_typequal );
	SRULE( prequaled_type, *prefix_typequal >> unqualed_type );
	SRULE(
		unqualed_type,
		( lparen >> postqualed_type > rparen )
		| struct_decl
		| ident
		);
	SRULE( prefix_typequal, access_typequal );
	SRULE( postfix_typequal, access_typequal | func_typequal | array_typequal );
	SRULE( func_typequal, lparen >> *param_typequal > rparen );
	SRULE( array_typequal, lsbracket > -expr > rsbracket );
	SRULE( param_typequal, declspec >> -ident >> -init );
	SRULE( access_typequal, kw_const | kw_uniform | kw_shared );
}

void grammars::set_inits()
{
	SRULE( init, STERM(equal) > c_style_init );
	SRULE( c_style_init, assignexpr | nullable_initlist );
	SRULE( paren_init, lparen >> -( assignexpr >> *(comma > assignexpr) ) > rparen );
	SRULE( nullable_initlist, lbrace >> -init_list > rbrace );
	SRULE( init_list, c_style_init >> *(comma > c_style_init) );
}

void grammars::set_stmts()
{
	SRULE(
		stmt,
		stmt_if
		| stmt_while | stmt_dowhile
		| stmt_for
		| stmt_switch
		| stmt_compound
		| stmt_flowctrl
		| labeled_stmt
		| stmt_decl
		| stmt_expr
		);
	SRULE( stmt_if, kw_if > lparen > expr > rparen > stmt > -(kw_else > stmt) );
	SRULE( stmt_while, kw_while > lparen > expr > rparen > stmt );
	SRULE( stmt_dowhile, kw_do > stmt > kw_while > lparen > expr > rparen > semicolon );
	SRULE( stmt_for, kw_for > for_looper > stmt );
	SRULE( stmt_switch, kw_switch > lparen > expr > rparen > stmt_compound );
	SRULE( stmt_expr, expr > semicolon );
	SRULE( stmt_decl, basic_decl );
	SRULE( stmt_compound, lbrace >> *stmt > rbrace );
	SRULE( stmt_flowctrl, stmt_break | stmt_continue | stmt_return );
	SRULE( stmt_break, kw_break > semicolon );
	SRULE( stmt_continue, kw_continue > semicolon );
	SRULE( stmt_return, kw_return >> -expr > semicolon );
	SRULE( labeled_stmt, (kw_default | (kw_case > expr) | ident) >> colon > stmt );
	SRULE( for_init_decl, stmt_decl | stmt_expr );
	SRULE( for_looper, lparen > for_init_decl > -expr > semicolon > -expr > rparen );
}

void grammars::set_terms()
{
	SRULE( lit_const,	lit_int | lit_float | lit_bool );
	
	SRULE( lit_int,		STERM2(lit_int,		"integer") );
	SRULE( lit_float,	STERM2(lit_float,	"float") );
	SRULE( lit_bool,	STERM2(lit_bool,	"bool") );

	SRULE( opadd,		STERM2(plus, "+") | STERM2(minus, "-") );
	SRULE( opassign,	STERM2(equal, "=") |
						STERM2(add_assign, "+=") | STERM2(sub_assign, "-=") |
						STERM2(mul_assign, "*=") | STERM2(div_assign, "/=") );
	SRULE( opmul,		STERM2(asterisk, "*") | STERM2(slash, "/") | STERM2(percent, "%") );
	SRULE( opshift,		STERM2(shift, "<< or >>") );
	SRULE( oprel,		STERM2(less_equal, "<=") | STERM2(greater_equal, ">=") |
						STERM2(labracket, "<") | STERM2(rabracket, ">") );
	SRULE( opequal,		STERM2(equal_to, "==") | STERM2(not_equal, "!=") );
	SRULE( opband,		STERM2(ampersand, "&") );
	SRULE( opbxor,		STERM2(caret, "^") );
	SRULE( opbor,		STERM2(vertical, "|") );
	SRULE( opland,		STERM2(logic_and, "&&") );
	SRULE( oplor,		STERM2(logic_or, "||") );
	SRULE( opmember,	STERM2(dot, ".") );
	
	SRULE( lparen,		STERM2(lparen, "(") );
	SRULE( rparen,		STERM2(rparen, ")") );
	SRULE( lsbracket,	STERM2(lsbracket, "[") );
	SRULE( rsbracket,	STERM2(rsbracket, "]") );
	SRULE( labracket,	STERM2(labracket, "<") );
	SRULE( rabracket,	STERM2(rabracket, ">") );
	SRULE( lbrace,		STERM2(lbrace, "{") );
	SRULE( rbrace,		STERM2(rbrace, "}") );
	SRULE( opinc,		STERM2(inc_dec, "++ or --") );
	SRULE( opunary,		STERM2(plus, "+") | STERM2(minus, "-") | STERM2(tilde, "~") | STERM2(exclamation, "!") );

	SRULE( kw_typedef,	STERM2(kw_typedef, "typedef") );
	SRULE( kw_struct,	STERM2(kw_struct, "struct") );
	SRULE( kw_uniform,	STERM2(kw_uniform, "uniform") );
	SRULE( kw_const,	STERM2(kw_const, "const") );
	SRULE( kw_shared,	STERM2(kw_shared, "shared") );
	SRULE( kw_break,	STERM2(kw_break, "break") );
	SRULE( kw_continue,	STERM2(kw_continue, "continue") );
	SRULE( kw_return,	STERM2(kw_return, "return") );
	SRULE( kw_switch,	STERM2(kw_switch, "switch") );
	SRULE( kw_case,		STERM2(kw_case, "case") );
	SRULE( kw_default,	STERM2(kw_default, "default") );
	SRULE( kw_if,		STERM2(kw_if, "if") );
	SRULE( kw_else,		STERM2(kw_else, "else") );
	SRULE( kw_for,		STERM2(kw_for, "for") );
	SRULE( kw_do,		STERM2(kw_do, "do") );
	SRULE( kw_while,	STERM2(kw_while, "while") );

	SRULE( question,	STERM2(question, "?") );
	SRULE( colon,		STERM2(colon, ":") );
	SRULE( comma,		STERM2(comma, ",") );
	SRULE( semicolon,	STERM2(semicolon, ";") );

	SRULE( ident,		STERM2(ident, "identifier") );
	SRULE( eof,			endholder() );
}

END_NS_SASL_PARSER();

