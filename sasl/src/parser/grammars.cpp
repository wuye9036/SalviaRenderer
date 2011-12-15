#include <sasl/include/parser/grammars.h>

#include <sasl/include/parser/lexer.h>

#define STERM( name ) terminal( lxr.get_id( #name ) )
#define SRULE( rule_name, def ) \
	rule_name = def;	\
	rule_name.name( #rule_name );

BEGIN_NS_SASL_PARSER();

grammars::grammars( lexer& lxr ):lxr(lxr){
	set_prog();
	set_decls();
	set_exprs();
	set_typespecs();
	set_inits();
	set_stmts();
	set_terms();
}

void grammars::set_prog(){
	SRULE(
		prog,
		*decl
		);
}

void grammars::set_decls()
{
	SRULE( decl, function_def | basic_decl );
	SRULE(
		basic_decl,
		STERM(semicolon) | 
		(
			( function_decl
			| struct_decl
			| typedef_decl
			| vardecl 
			) > STERM(semicolon)
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
	SRULE( typecastedexpr, ( lparen >> declspec > rparen ) > expr );
	SRULE( unaryexpr, postexpr | unariedexpr );
	SRULE( unariedexpr, (opinc | opunary) > castexpr );
	SRULE( postexpr, pmexpr >> *(idxexpr | callexpr | memexpr | opinc) );
	SRULE( idxexpr, lsbracket >> expr > rsbracket );
	SRULE( callexpr, lparen >> -exprlst > rparen );
	SRULE( memexpr, opmember > ident );
	SRULE( pmexpr, lit_const | ident | parenexpr );
	SRULE( parenexpr, lparen >> expr > rparen );
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
	SRULE( stmt_decl, decl );
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
	
	SRULE( lit_int,		STERM(lit_int) );
	SRULE( lit_float,	STERM(lit_float) );
	SRULE( lit_bool,	STERM(lit_bool) );

	SRULE( opadd,		STERM(plus) | STERM(minus) );
	SRULE( opassign,	STERM(equal) |
						STERM(add_assign) | STERM(sub_assign) |
						STERM(mul_assign) | STERM(div_assign) );
	SRULE( opmul,		STERM(asterisk) | STERM(slash) | STERM(percent) );
	SRULE( opshift,		STERM(shift) );
	SRULE( oprel,		STERM(less_equal) | STERM(greater_equal) |
						STERM(labracket) | STERM(rabracket) );
	SRULE( opequal,		STERM(equal_to) | STERM(not_equal) );
	SRULE( opband,		STERM(ampersand) );
	SRULE( opbxor,		STERM(caret) );
	SRULE( opbor,		STERM(vertical) );
	SRULE( opland,		STERM(logic_and) );
	SRULE( oplor,		STERM(logic_or) );
	SRULE( opmember,	STERM(dot) );
	
	SRULE( lparen,		STERM(lparen) );
	SRULE( rparen,		STERM(rparen) );
	SRULE( lsbracket,	STERM(lsbracket) );
	SRULE( rsbracket,	STERM(rsbracket) );
	SRULE( labracket,	STERM(labracket) );
	SRULE( rabracket,	STERM(rabracket) );
	SRULE( lbrace,		STERM(lbrace) );
	SRULE( rbrace,		STERM(rbrace) );
	SRULE( opinc,		STERM(inc_dec) );
	SRULE( opunary,		STERM(plus) | STERM(minus) | STERM(tilde) | STERM(exclamation) );

	SRULE( kw_typedef,	STERM(kw_typedef) );
	SRULE( kw_struct,	STERM(kw_struct) );
	SRULE( kw_uniform,	STERM(kw_uniform) );
	SRULE( kw_const,	STERM(kw_const) );
	SRULE( kw_shared,	STERM(kw_shared) );
	SRULE( kw_break,	STERM(kw_break) );
	SRULE( kw_continue,	STERM(kw_continue) );
	SRULE( kw_return,	STERM(kw_return) );
	SRULE( kw_switch,	STERM(kw_switch) );
	SRULE( kw_case,		STERM(kw_case) );
	SRULE( kw_default,	STERM(kw_default) );
	SRULE( kw_if,		STERM(kw_if) );
	SRULE( kw_else,		STERM(kw_else) );
	SRULE( kw_for,		STERM(kw_for) );
	SRULE( kw_do,		STERM(kw_do) );
	SRULE( kw_while,	STERM(kw_while) );

	SRULE( question,	STERM(question) );
	SRULE( colon,		STERM(colon) );
	SRULE( comma,		STERM(comma) );
	SRULE( semicolon,	STERM(semicolon) );

	SRULE( ident,		STERM(ident) );
	SRULE( eof,			endholder() );
}

END_NS_SASL_PARSER();

