#include <sasl/include/syntax_tree/syntax_tree_builder.h>

#include <sasl/include/syntax_tree/utility.h>
#include <sasl/include/syntax_tree/node_creation.h>

#include <sasl/include/parser/lexer.h>
#include <sasl/include/parser/generator.h>
#include <sasl/include/parser/grammars.h>
#include <sasl/enums/enums_utility.h>

#include <eflib/include/diagnostics/assert.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/foreach.hpp>
#include <eflib/include/platform/boost_end.h>

using namespace sasl::utility;

using sasl::common::token_t;

using sasl::parser::attribute;
using sasl::parser::grammars;
using sasl::parser::lexer;
using sasl::parser::queuer_attribute;
using sasl::parser::selector_attribute;
using sasl::parser::sequence_attribute;
using sasl::parser::terminal_attribute;

using boost::shared_dynamic_cast;
using boost::shared_polymorphic_cast;
using boost::shared_ptr;
using boost::unordered_map;

using std::make_pair;
using std::string;
using std::vector;

BEGIN_NS_SASL_SYNTAX_TREE();

#define SASL_TYPED_ATTRIBUTE( type, dest, src ) \
	shared_ptr< type > dest = shared_polymorphic_cast<type>(src);

#define SASL_DYNCAST_ATTRIBUTE( type, dest, src ) \
	shared_ptr< type > dest = shared_dynamic_cast<type>(src);

#define SASL_SWITCH_RULE( attr ) \
	{ \
		intptr_t rule_attr_id = (attr)->rule_id(); \
		if( rule_attr_id < -1 ) { \
			EFLIB_ASSERT( rule_attr_id >= -1, "Rule ID must be in [-1, 2^31-1]" ); \
		}

#define SASL_CASE_RULE( rule ) else if ( rule_attr_id == g.rule.id() )
#define SASL_DEFAULT() else
#define SASL_END_SWITCH_RULE() }


#define INSERT_INTO_BTCACHE( litname, enum_code ) \
	{	\
		shared_ptr<builtin_type> bt = create_node<builtin_type>( token_t::null(), token_t::null() );	\
		bt->tycode = builtin_types::enum_code;	\
		bt_cache.insert( make_pair( std::string( #litname ), bt ) );	\
	}

#define INSERT_VECTOR_INTO_BTCACHE( component_type, dim, enum_code ) \
	{	\
		shared_ptr<builtin_type> bt = create_node<builtin_type>( token_t::null(), token_t::null() );	\
		bt->tycode = vector_of( builtin_types::enum_code, dim );	\
		bt_cache.insert( make_pair( string( #component_type ) + char_tbl[dim], bt ) );	\
	}

#define INSERT_MATRIX_INTO_BTCACHE( component_type, vsize, vcnt, enum_code ) \
	{	\
		shared_ptr<builtin_type> bt = create_node<builtin_type>( token_t::null(), token_t::null() );	\
		bt->tycode = matrix_of( builtin_types::enum_code, vsize, vcnt );	\
		bt_cache.insert( make_pair( string( #component_type ) + char_tbl[vcnt] + "x" + char_tbl[vsize], bt ) );	\
	}

syntax_tree_builder::syntax_tree_builder( lexer& l, grammars& g ): l(l), g(g){}

void syntax_tree_builder::initialize_bt_cache(){
	if( bt_cache.empty() ){

		char const char_tbl[] = { '0', '1', '2', '3', '4' };

		INSERT_INTO_BTCACHE( char,    _sint8   );
		INSERT_INTO_BTCACHE( sbyte,    _sint8   );
		INSERT_INTO_BTCACHE( int8_t,   _sint8   );
		INSERT_INTO_BTCACHE( uchar,    _sint8   );
		INSERT_INTO_BTCACHE( ubyte,    _uint8   );
		INSERT_INTO_BTCACHE( uint8_t,  _uint8   );
		INSERT_INTO_BTCACHE( short,    _sint16  );
		INSERT_INTO_BTCACHE( int16_t,  _sint16  );
		INSERT_INTO_BTCACHE( ushort,   _uint16  );
		INSERT_INTO_BTCACHE( uint16_t, _uint16  );
		INSERT_INTO_BTCACHE( int,      _sint32  );
		INSERT_INTO_BTCACHE( int32_t,  _sint32  );
		INSERT_INTO_BTCACHE( uint,     _uint32  );
		INSERT_INTO_BTCACHE( uint32_t, _uint32  );
		INSERT_INTO_BTCACHE( long,     _sint64  );
		INSERT_INTO_BTCACHE( int64_t,  _sint64  );
		INSERT_INTO_BTCACHE( ulong,    _uint64  );
		INSERT_INTO_BTCACHE( uint64_t, _uint64  );

		INSERT_INTO_BTCACHE( float,    _float   );
		INSERT_INTO_BTCACHE( double,   _double  );
		INSERT_INTO_BTCACHE( bool,     _boolean );
		INSERT_INTO_BTCACHE( void,     _void    );

		for( int dim0 = 1; dim0 <= 4; ++dim0 ){
			INSERT_VECTOR_INTO_BTCACHE( char,	dim0, _sint8 );
			INSERT_VECTOR_INTO_BTCACHE( uchar,	dim0, _uint8 );
			INSERT_VECTOR_INTO_BTCACHE( short,	dim0, _sint16 );
			INSERT_VECTOR_INTO_BTCACHE( ushort,	dim0, _uint16 );
			INSERT_VECTOR_INTO_BTCACHE( int,	dim0, _sint32 );
			INSERT_VECTOR_INTO_BTCACHE( uint,	dim0, _uint32 );
			INSERT_VECTOR_INTO_BTCACHE( long,	dim0, _sint64 );
			INSERT_VECTOR_INTO_BTCACHE( ulong,	dim0, _uint64 );
			INSERT_VECTOR_INTO_BTCACHE( float,	dim0, _float );
			INSERT_VECTOR_INTO_BTCACHE( double,	dim0, _double );
			INSERT_VECTOR_INTO_BTCACHE( bool,	dim0, _boolean );

			for( int dim1 = 1; dim1 <= 4; ++dim1 ){
				INSERT_MATRIX_INTO_BTCACHE( char,	dim0, dim1, _sint8 );
				INSERT_MATRIX_INTO_BTCACHE( uchar,	dim0, dim1, _uint8 );
				INSERT_MATRIX_INTO_BTCACHE( short,	dim0, dim1, _sint16 );
				INSERT_MATRIX_INTO_BTCACHE( ushort,	dim0, dim1, _uint16 );
				INSERT_MATRIX_INTO_BTCACHE( int,	dim0, dim1, _sint32 );
				INSERT_MATRIX_INTO_BTCACHE( uint,	dim0, dim1, _uint32 );
				INSERT_MATRIX_INTO_BTCACHE( long,	dim0, dim1, _sint64 );
				INSERT_MATRIX_INTO_BTCACHE( ulong,	dim0, dim1, _uint64 );
				INSERT_MATRIX_INTO_BTCACHE( float,	dim0, dim1, _float );
				INSERT_MATRIX_INTO_BTCACHE( double,	dim0, dim1, _double );
				INSERT_MATRIX_INTO_BTCACHE( bool,	dim0, dim1, _boolean );
			}
		}
	}
}

shared_ptr<program> syntax_tree_builder::build_prog( shared_ptr< attribute > attr )
{
	shared_ptr<program> ret;
	
	SASL_TYPED_ATTRIBUTE(sequence_attribute, typed_attr, attr->child(0));

	if( typed_attr ){
		ret = create_node<program>("prog");

		BOOST_FOREACH( shared_ptr<attribute> decl_attr, typed_attr->attrs ){
			vector< shared_ptr<declaration> > decls = build_decl( decl_attr );
			ret->decls.insert( ret->decls.end(), decls.begin(), decls.end() );
		}
	}

	return ret;
}

vector< shared_ptr<declaration> > syntax_tree_builder::build_decl( shared_ptr<attribute> attr )
{
	vector< shared_ptr<declaration> > ret;
	
	SASL_TYPED_ATTRIBUTE(selector_attribute, typed_attr, attr);
	EFLIB_ASSERT_AND_IF( typed_attr->selected_idx >= 0, "Attribute error: least one branch was selected." ){
		return ret;
	}
	
	SASL_SWITCH_RULE( typed_attr->attr )
		SASL_CASE_RULE( basic_decl ){
			return build_basic_decl( typed_attr->attr );
		}
		SASL_CASE_RULE( function_def ){
			ret.push_back( build_fndef(typed_attr->attr) );
			return ret;
		}
		SASL_CASE_RULE( function_decl ){
			ret.push_back( build_fndecl( typed_attr->attr->child(0) ) );
			return ret;
		}
	SASL_END_SWITCH_RULE();

	return ret;
}

shared_ptr<function_type> syntax_tree_builder::build_fndef( shared_ptr<attribute> attr ){
	SASL_TYPED_ATTRIBUTE( queuer_attribute, typed_attr, attr );
	shared_ptr<function_type> ret = build_fndecl(typed_attr->attrs[0]);
	ret->body = build_stmt_compound( typed_attr->attrs[1] );

	return ret;
}

vector< shared_ptr<declaration> > syntax_tree_builder::build_basic_decl( shared_ptr<attribute> attr ){
	vector< shared_ptr<declaration> > ret;

	SASL_TYPED_ATTRIBUTE(selector_attribute, typed_attr, attr);

	SASL_DYNCAST_ATTRIBUTE( terminal_attribute, semicolon_attr, typed_attr->attr );
	if( semicolon_attr ){
		// ";" only
		return ret;
	}

	SASL_TYPED_ATTRIBUTE( queuer_attribute, not_empty_typed_attr, typed_attr->attr );
	EFLIB_ASSERT_AND_IF( not_empty_typed_attr->attrs.size() == 2, "Basic declaration must not a empty queuer." ){
		return ret;
	}

	SASL_TYPED_ATTRIBUTE( selector_attribute, typed_decl_attr, not_empty_typed_attr->attrs[0] );
	
	SASL_SWITCH_RULE( typed_decl_attr->attr )
		SASL_CASE_RULE( vardecl ){
			vector< shared_ptr<variable_declaration> > decls
				= build_vardecl(typed_decl_attr->attr);
			ret.insert( ret.end(), decls.begin(), decls.end() );
		}
		SASL_CASE_RULE( struct_decl ){
			ret.push_back( build_struct( typed_decl_attr->attr ) );
			return ret;
		}
		SASL_CASE_RULE( typedef_decl ){
			EFLIB_ASSERT_UNIMPLEMENTED();
		}
		SASL_DEFAULT(){
			assert(!"Unknown declaration!");
		}
	SASL_END_SWITCH_RULE();

	return ret;
}

vector< shared_ptr<variable_declaration> > syntax_tree_builder::build_vardecl( shared_ptr<attribute> attr ){
	vector< shared_ptr<variable_declaration> > ret;

	SASL_TYPED_ATTRIBUTE( queuer_attribute, typed_attr, attr );

	assert( typed_attr->attrs.size() == 2 );

	shared_ptr<variable_declaration> decl
		= create_node<variable_declaration>( attr->token_beg(), attr->token_end() );
	ret.push_back(decl);
	decl->type_info = build_typespec( typed_attr->attrs[0] );
	decl->declarators = build_declarators( typed_attr->attrs[1], decl->type_info, ret );
	
	return ret;
}

shared_ptr<function_type> syntax_tree_builder::build_fndecl( shared_ptr<attribute> attr ){
	shared_ptr<function_type> ret = create_node<function_type>( attr->token_beg(), attr->token_end() );

	SASL_TYPED_ATTRIBUTE( queuer_attribute, typed_attr, attr );
	ret->retval_type = build_typespec( typed_attr->attrs[0] );

	SASL_TYPED_ATTRIBUTE( terminal_attribute, name_attr, typed_attr->attrs[1] );
	ret->name = name_attr->tok;
	
	SASL_TYPED_ATTRIBUTE( queuer_attribute, paren_params_attr, typed_attr->attrs[2] );
	SASL_TYPED_ATTRIBUTE( sequence_attribute, optional_params_attr, paren_params_attr->attrs[1] );

	if( !optional_params_attr->attrs.empty() ){
		SASL_TYPED_ATTRIBUTE( queuer_attribute, params_attr, optional_params_attr->attrs[0] );
		
		// params_attr: param >> *( comma > param )

		// Head param
		ret->params.push_back( build_param( params_attr->child(0) ) );

		// Follows
		SASL_TYPED_ATTRIBUTE( sequence_attribute, follow_params_attr, params_attr->child(1) );
		BOOST_FOREACH( shared_ptr<attribute> comma_param_attr, follow_params_attr->attrs )
		{
			ret->params.push_back(
				build_param( comma_param_attr->child(1) )
				);
		}	
	}
	
	build_semantic( typed_attr->attrs[3], ret->semantic, ret->semantic_index );

	return ret;
}

shared_ptr<parameter> syntax_tree_builder::build_param( shared_ptr<attribute> attr ){
	shared_ptr<parameter> ret = create_node<parameter>( attr->token_beg(), attr->token_end() );
	
	SASL_TYPED_ATTRIBUTE( queuer_attribute, typed_attr, attr );
	ret->param_type = build_typespec( typed_attr->attrs[0] );

	SASL_TYPED_ATTRIBUTE( sequence_attribute, optional_ident, typed_attr->attrs[1] );
	build_semantic( typed_attr->attrs[2], ret->semantic, ret->semantic_index );

	SASL_TYPED_ATTRIBUTE( sequence_attribute, optional_init, typed_attr->attrs[3] );
	if( !optional_ident->attrs.empty() ){
		SASL_TYPED_ATTRIBUTE( terminal_attribute, ident_attr, optional_ident->attrs[0] );
		ret->name = ident_attr->tok;
	}

	if( !optional_init->attrs.empty() ){
		ret->init = build_init( optional_init->attrs[0] );
	}

	return ret;
}

shared_ptr<struct_type> syntax_tree_builder::build_struct( shared_ptr<attribute> attr ){
	shared_ptr<struct_type> ret = create_node<struct_type>( attr->token_beg(), attr->token_end() );

	SASL_TYPED_ATTRIBUTE( queuer_attribute, typed_attr, attr );
	SASL_TYPED_ATTRIBUTE( selector_attribute, body_attr, typed_attr->attrs[1] );

	SASL_SWITCH_RULE( body_attr->attr )
		SASL_CASE_RULE( struct_body ){
			build_struct_body( body_attr->attr, ret );
		}
		SASL_CASE_RULE( named_struct_body ){
			SASL_TYPED_ATTRIBUTE( queuer_attribute, named_body_attr, body_attr->attr );
			SASL_TYPED_ATTRIBUTE( terminal_attribute, name_attr, named_body_attr->attrs[0] );
			ret->name = name_attr->tok;
			SASL_TYPED_ATTRIBUTE( sequence_attribute, opt_body_attr, named_body_attr->attrs[1] );
			if( !opt_body_attr->attrs.empty() ){
				build_struct_body( opt_body_attr->attrs[0], ret );
			}
		}
	SASL_END_SWITCH_RULE()

	return ret;
}

void syntax_tree_builder::build_struct_body( shared_ptr<attribute> attr, shared_ptr<struct_type> out ){
	assert( out );
	SASL_TYPED_ATTRIBUTE( queuer_attribute, typed_attr, attr );
	SASL_TYPED_ATTRIBUTE( sequence_attribute, decls_attr, typed_attr->attrs[1] );

	out->has_body = true;
	BOOST_FOREACH( shared_ptr<attribute> const& decl_attr, decls_attr->attrs )
	{
		vector< shared_ptr<declaration> > decls = build_decl(decl_attr);
		out->decls.insert( out->decls.end(), decls.begin(), decls.end() );
	}
}

shared_ptr<expression> syntax_tree_builder::build_expr( shared_ptr<attribute> attr ){
	if( !attr ){
		return shared_ptr<expression>();
	}

	shared_ptr<expression_list> ret = build_exprlst( attr );
	if ( ret->exprs.size() == 1 ){
		return ret->exprs[0];
	}
	return ret;
}

shared_ptr<expression_list> syntax_tree_builder::build_exprlst( shared_ptr<attribute> attr ){
	shared_ptr<expression_list> ret = create_node<expression_list>( attr->token_beg(), attr->token_end() );

	SASL_TYPED_ATTRIBUTE( queuer_attribute, typed_attr, attr );
	ret->exprs.push_back( build_assignexpr( typed_attr->attrs[0] ) );

	SASL_TYPED_ATTRIBUTE( sequence_attribute, follows, typed_attr->attrs[1] );
	BOOST_FOREACH( shared_ptr<attribute> follow_pair, follows->attrs ){
		SASL_TYPED_ATTRIBUTE( queuer_attribute, typed_follow_pair, follow_pair );
		ret->exprs.push_back( build_assignexpr( typed_follow_pair->attrs[1] ) );
	}

	return ret;
}

operators syntax_tree_builder::build_prefix_op(shared_ptr<attribute> attr, shared_ptr<token_t>& op_tok)
{
	SASL_TYPED_ATTRIBUTE( terminal_attribute, tok_attr, attr );

	assert( tok_attr );

	op_tok = tok_attr->tok;
	std::string const& op_str = op_tok->str;
	char op_chars[4] = {'\0', '\0', '\0', '\0'};
	for( size_t i = 0; i < op_str.length(); ++i ){ op_chars[i] = op_str[i]; }

	switch( op_chars[0] ){
	case '+':
		return op_chars[1] == '+' ? operators::prefix_incr : operators::positive;
	case '-':
		return op_chars[1] == '-' ? operators::prefix_decr : operators::negative;
	case '!':
		return operators::logic_not;
	case '~':
		return operators::bit_not;
	}

	string assertion("Unsupported operator: ");
	assertion += op_str;
	EFLIB_ASSERT_UNIMPLEMENTED0( assertion.c_str() );

	return operators::none;
}

operators syntax_tree_builder::build_postfix_op(shared_ptr<attribute> attr, shared_ptr<token_t>& op_tok)
{
	SASL_TYPED_ATTRIBUTE(terminal_attribute, tok_attr, attr);
	op_tok = tok_attr->tok;
	switch(op_tok->str[0])
	{
	case '+': // ++
		return operators::postfix_incr;
	case '-': // --
		return operators::postfix_decr;
	}

	string assertion("Unsupported operator: ");
	assertion += op_tok->str;
	EFLIB_ASSERT_UNIMPLEMENTED0( assertion.c_str() );

	return operators::none;
}

/* Build assignment expression tree.
* Example:
*    Expression: a = b = c
* Generated tree:
*        expr
*        /  \
*     expr0  a
*     /  \
*    c    b
* */
shared_ptr<expression> syntax_tree_builder::build_assignexpr( shared_ptr<attribute> attr ){

	// Make expression list and operators list.
	vector< shared_ptr<expression> > exprs;
	vector< operators > ops;
	vector< shared_ptr<token_t> > op_tokens;

	exprs.push_back( build_rhsexpr( attr->child(0) ) );
	SASL_TYPED_ATTRIBUTE( sequence_attribute, follows, attr->child(1) );
	BOOST_FOREACH( shared_ptr<attribute> follow_pair, follows->attrs ){
		exprs.push_back( 
			build_rhsexpr( follow_pair->child(1) )
			);
		shared_ptr<attribute> op_attr = follow_pair->child(0);
		shared_ptr<token_t> op_token;
		ops.push_back( build_binop(op_attr, op_token) );
		op_tokens.push_back(op_token);
	}

	// Build tree
	shared_ptr<expression> root;

	BOOST_REVERSE_FOREACH( shared_ptr<expression> const& expr, exprs ){
		if( !root ){
			root = expr;
		} else {
			shared_ptr<binary_expression> new_root
				= create_node<binary_expression>( expr->token_begin(), root->token_end() );
			new_root->left_expr = root;
			new_root->right_expr = expr;
			new_root->op = ops.back();
			new_root->op_token = op_tokens.back();
			op_tokens.pop_back();
			ops.pop_back();
			root = new_root;
		}
	}

	return root;
}

shared_ptr<expression> syntax_tree_builder::build_lcomb_expr( shared_ptr<attribute> attr ){

	SASL_TYPED_ATTRIBUTE( queuer_attribute, typed_attr, attr );
	shared_ptr<expression> lexpr = dispatch_lcomb_expr( typed_attr->attrs[0] );

	shared_ptr<binary_expression> binexpr;
	SASL_TYPED_ATTRIBUTE( sequence_attribute, follows, typed_attr->attrs[1] );
	BOOST_FOREACH( shared_ptr<attribute> follow_pair, follows->attrs ){
		assert( dynamic_cast<queuer_attribute*>(follow_pair.get()) != NULL );
		binexpr = create_node<binary_expression>( lexpr->token_begin(), follow_pair->child(1)->token_end() );
		binexpr->left_expr = lexpr;
		shared_ptr<token_t> op_token;
		binexpr->op = build_binop(follow_pair->child(0), op_token);
		binexpr->op_token = op_token;
		binexpr->right_expr = dispatch_lcomb_expr(follow_pair->child(1));
		lexpr = binexpr;
	}

	return lexpr;
}

shared_ptr<expression> syntax_tree_builder::dispatch_lcomb_expr( shared_ptr<attribute> attr ){
	shared_ptr<expression> ret;
	SASL_SWITCH_RULE( attr )
		SASL_CASE_RULE( lorexpr ){
			return build_lcomb_expr(attr);
		}
		SASL_CASE_RULE( landexpr ){
			return build_lcomb_expr(attr);
		}
		SASL_CASE_RULE( borexpr ){
			return build_lcomb_expr(attr);
		}
		SASL_CASE_RULE( bxorexpr ){
			return build_lcomb_expr(attr);
		}
		SASL_CASE_RULE( bandexpr ){
			return build_lcomb_expr(attr);
		}
		SASL_CASE_RULE( eqlexpr ){
			return build_lcomb_expr(attr);
		}
		SASL_CASE_RULE( relexpr ){
			return build_lcomb_expr(attr);
		}
		SASL_CASE_RULE( shfexpr ){
			return build_lcomb_expr(attr);
		}
		SASL_CASE_RULE( addexpr ){
			return build_lcomb_expr(attr);
		}
		SASL_CASE_RULE( mulexpr ){
			return build_lcomb_expr(attr);
		}
		SASL_CASE_RULE( castexpr ){
			return build_castexpr(attr);
		}
		SASL_DEFAULT(){
			assert( !"Wrong rule was invoked!" );
		}
	SASL_END_SWITCH_RULE();

	return ret;
}

shared_ptr<expression> syntax_tree_builder::build_rhsexpr( shared_ptr<attribute> attr ){
	SASL_TYPED_ATTRIBUTE( selector_attribute, typed_attr, attr );
	SASL_SWITCH_RULE( typed_attr->attr )
		SASL_CASE_RULE( condexpr ){
			return build_condexpr( typed_attr->attr );
		}
		SASL_CASE_RULE( lorexpr ){
			return build_lcomb_expr( typed_attr->attr );
		}
		SASL_DEFAULT(){
			assert( !"Error was happened!" );
		}
	SASL_END_SWITCH_RULE();

	return shared_ptr<expression>();
}

shared_ptr<expression> syntax_tree_builder::build_condexpr( shared_ptr<attribute> attr ){

	shared_ptr<cond_expression> ret = create_node<cond_expression>( attr->token_beg(), attr->token_end() );

	shared_ptr<attribute> cond_attr = attr->child(0);
	shared_ptr<attribute> yesno_attr = attr->child(1);

	ret->cond_expr = build_lcomb_expr( cond_attr );
	ret->yes_expr = build_expr( yesno_attr->child(1) );
	ret->no_expr = build_assignexpr( yesno_attr->child(3) );

	if( !(ret->cond_expr && ret->yes_expr && ret->no_expr) ){
		assert( false );
		ret.reset();
	}

	return ret;
}

shared_ptr<expression> syntax_tree_builder::build_castexpr( shared_ptr<attribute> attr ){
	SASL_TYPED_ATTRIBUTE( selector_attribute, typed_attr, attr );
	SASL_SWITCH_RULE( typed_attr->attr )
		SASL_CASE_RULE( unaryexpr ){
			return build_unaryexpr( typed_attr->attr );
		}
		SASL_CASE_RULE( typecastedexpr ){
			return build_typecastedexpr( typed_attr->attr );
		}
		SASL_DEFAULT(){
			assert( !"Error was happened!" );
		}
	SASL_END_SWITCH_RULE();

	return shared_ptr<expression>();
}

shared_ptr<expression> syntax_tree_builder::build_unaryexpr( shared_ptr<attribute> attr ){
	SASL_TYPED_ATTRIBUTE( selector_attribute, typed_attr, attr );
	SASL_SWITCH_RULE( typed_attr->attr )
		SASL_CASE_RULE( unariedexpr ){
			return build_unariedexpr( typed_attr->attr );
		}
		SASL_CASE_RULE( postexpr ){
			return build_postexpr( typed_attr->attr );
		}
		SASL_DEFAULT(){
			assert( !"Error was happened!" );
		}
	SASL_END_SWITCH_RULE();

	return shared_ptr<expression>();
}

shared_ptr<unary_expression> syntax_tree_builder::build_unariedexpr( shared_ptr<attribute> attr ){
	shared_ptr<attribute> op_attr = attr->child(0)->child(0);
	if( op_attr->rule_id() == g.opunary.id() ) {
		op_attr = op_attr->child(0);
	}
	shared_ptr<attribute> expr_attr = attr->child(1);

	assert( op_attr );
	assert( expr_attr );

	shared_ptr<expression> expr = build_castexpr(expr_attr);
	assert( expr );

	shared_ptr<unary_expression> ret = create_node<unary_expression>( attr->token_beg(), attr->token_end() );
	ret->expr = expr;
	shared_ptr<token_t> op_token;
	ret->op = build_prefix_op(op_attr, op_token);
	ret->op_token = op_token;

	return ret;
}

shared_ptr<expression> syntax_tree_builder::build_postexpr( shared_ptr<attribute> attr ){
	shared_ptr<expression> ret = build_pmexpr( attr->child(0) );

	SASL_TYPED_ATTRIBUTE( sequence_attribute, postfix_attrs, attr->child(1) );
	BOOST_FOREACH( shared_ptr<attribute> postfix_attr, postfix_attrs->attrs ){
		shared_ptr<attribute> expr_attr = postfix_attr->child(0);
		SASL_SWITCH_RULE( expr_attr )
			SASL_CASE_RULE( idxexpr ){
				ret = build_indexexpr( expr_attr->child(1), ret );
			}
			SASL_CASE_RULE( callexpr ){
				ret = build_callexpr( expr_attr->child(1), ret );
			}
			SASL_CASE_RULE( memexpr ){
				ret = build_memexpr(expr_attr, ret);
			}
			SASL_CASE_RULE( opinc ){
				shared_ptr<unary_expression> ret_unary = create_node<unary_expression>( ret->token_begin(), expr_attr->token_end() );
				shared_ptr<token_t> op_token;
				ret_unary->op = build_postfix_op(expr_attr, op_token);
				ret_unary->op_token = op_token;
				ret_unary->expr = ret;
				ret = ret_unary;
			}
		SASL_END_SWITCH_RULE();
	}

	return ret;
}

shared_ptr<expression> syntax_tree_builder::build_callexpr(
	shared_ptr<attribute> attr,
	shared_ptr<expression> expr )
{
	shared_ptr<call_expression> ret = create_node<call_expression>( expr->token_begin(), attr->token_end() );
	ret->expr = expr;

	SASL_TYPED_ATTRIBUTE( sequence_attribute, optional_args, attr );
	if( !optional_args->attrs.empty() )
	{
		shared_ptr<expression_list> arglst = build_exprlst( attr->child(0) );
		ret->args = arglst->exprs;
	}
	
	return ret;
}


shared_ptr<expression> syntax_tree_builder::build_indexexpr(
	shared_ptr<attribute> attr,
	shared_ptr<expression> expr )
{
	shared_ptr<index_expression> ret = create_node<index_expression>(expr->token_begin(), attr->token_end());
	ret->expr = expr;
	ret->index_expr = build_expr(attr);
	return ret;
}

shared_ptr<expression> syntax_tree_builder::build_memexpr(
	shared_ptr<attribute> attr,
	shared_ptr<expression> expr )
{
	shared_ptr<member_expression> ret = create_node<member_expression>( expr->token_begin(), attr->token_end() );
	ret->expr = expr;
	SASL_TYPED_ATTRIBUTE( terminal_attribute, mem_attr, attr->child(1) );
	ret->member = mem_attr->tok;

	return ret;
}

shared_ptr<expression> syntax_tree_builder::build_pmexpr( shared_ptr<attribute> attr ){
	SASL_TYPED_ATTRIBUTE( selector_attribute, typed_attr, attr );
	SASL_SWITCH_RULE( typed_attr->attr )
		SASL_CASE_RULE( lit_const ){
			SASL_TYPED_ATTRIBUTE( terminal_attribute, const_attr, typed_attr->attr->child(0) );
			shared_ptr<constant_expression> ret = create_node<constant_expression>( const_attr->tok, const_attr->tok );
			ret->value_tok = const_attr->tok;
			SASL_SWITCH_RULE( const_attr )
				SASL_CASE_RULE( lit_int ){
					ret->ctype = literal_classifications::integer;
				}
				SASL_CASE_RULE( lit_float ){
					ret->ctype = literal_classifications::real;
				}
				SASL_CASE_RULE( lit_bool ){
					ret->ctype = literal_classifications::boolean;
				}
			SASL_END_SWITCH_RULE();
			return ret;
		}
		SASL_CASE_RULE( ident ){
			SASL_TYPED_ATTRIBUTE( terminal_attribute, var_attr, typed_attr->attr );
			shared_ptr<variable_expression> varexpr = create_node<variable_expression>( var_attr->tok, var_attr->tok );
			varexpr->var_name = var_attr->tok;
			return varexpr;
		}
		SASL_CASE_RULE( parenexpr ){
			return build_expr( typed_attr->attr->child(1) );
		}
	SASL_END_SWITCH_RULE();

	return shared_ptr<expression>();
}

shared_ptr<cast_expression> syntax_tree_builder::build_typecastedexpr( shared_ptr<attribute> attr ){
	EFLIB_ASSERT_UNIMPLEMENTED();
	return shared_ptr<cast_expression>();
}

shared_ptr<tynode> syntax_tree_builder::build_typespec( shared_ptr<attribute> attr ){
	return build_postqualedtype( attr );
}

vector< shared_ptr<declarator> > syntax_tree_builder::build_declarators(
	shared_ptr<attribute> attr, 
	shared_ptr<tynode> tyn,
	vector< shared_ptr<variable_declaration> >& decls  )
{
	SASL_TYPED_ATTRIBUTE( queuer_attribute, typed_attr, attr );

	vector< shared_ptr<declarator> > ret;
	build_initdecl(typed_attr->attrs[0], tyn, ret, decls);

	SASL_TYPED_ATTRIBUTE( sequence_attribute, follows, typed_attr->attrs[1] );
	BOOST_FOREACH( shared_ptr<attribute> follow_attr, follows->attrs ){
		SASL_TYPED_ATTRIBUTE( queuer_attribute, follow_pair, follow_attr );
		build_initdecl(follow_pair->attrs[1], tyn, ret, decls);
	}

	return ret;
}


shared_ptr<tynode> syntax_tree_builder::build_unqualedtype( shared_ptr<attribute> attr ){
	shared_ptr<tynode> ret;

	SASL_TYPED_ATTRIBUTE( selector_attribute, typed_attr, attr );

	SASL_SWITCH_RULE( typed_attr->attr )
		SASL_CASE_RULE( ident ){
			
			shared_ptr<builtin_type> bt = get_builtin( typed_attr->attr );
			if( bt ) { return bt; }

			SASL_TYPED_ATTRIBUTE( terminal_attribute, ident_attr, typed_attr->attr );
			shared_ptr<alias_type> type_ident = create_node<alias_type>( ident_attr->tok, ident_attr->tok );
			type_ident->alias = ident_attr->tok;

			return type_ident;
		}
		SASL_CASE_RULE( struct_decl ){
			EFLIB_ASSERT_UNIMPLEMENTED();
			return ret;
		}
		SASL_DEFAULT(){
			EFLIB_ASSERT_UNIMPLEMENTED();
		}
	SASL_END_SWITCH_RULE();

	return ret;
}

shared_ptr<tynode> syntax_tree_builder::build_prequaledtype( shared_ptr<attribute> attr ){
	SASL_TYPED_ATTRIBUTE( queuer_attribute, typed_attr, attr );
	shared_ptr<tynode> ret_type = build_unqualedtype( typed_attr->attrs[1] );

	SASL_TYPED_ATTRIBUTE( sequence_attribute, quals_attr, typed_attr->attrs[0] );
	BOOST_FOREACH( shared_ptr<attribute> qual_attr, quals_attr->attrs ){
		ret_type = bind_typequal( attr, ret_type );
	}

	return ret_type;
}

shared_ptr<tynode> syntax_tree_builder::build_postqualedtype( shared_ptr<attribute> attr )
{
	SASL_TYPED_ATTRIBUTE( queuer_attribute, typed_attr, attr );
	shared_ptr<tynode> ret_type = build_prequaledtype( typed_attr->attrs[0] );

	SASL_TYPED_ATTRIBUTE( sequence_attribute, quals_attr, typed_attr->attrs[1] );
	BOOST_FOREACH( shared_ptr<attribute> qual_attr, quals_attr->attrs ){
		ret_type = bind_typequal( ret_type, attr );
	}

	return ret_type;
}

shared_ptr<initializer> syntax_tree_builder::build_init( shared_ptr<attribute> attr ){
	shared_ptr<initializer> ret;
	shared_ptr<attribute> init_body_attr = attr->child(1)->child(0);

	SASL_SWITCH_RULE( init_body_attr )
		SASL_CASE_RULE( assignexpr ){
			shared_ptr<expression_initializer> expr_init
				= create_node<expression_initializer>( attr->token_beg(), attr->token_end() );
			expr_init->init_expr = build_assignexpr( init_body_attr );
			ret = expr_init;
		}
		SASL_CASE_RULE( nullable_initlist ){
			EFLIB_ASSERT_UNIMPLEMENTED();
		}
	SASL_END_SWITCH_RULE();

	return ret;
}

shared_ptr<statement> syntax_tree_builder::build_stmt( shared_ptr<attribute> attr ){
	shared_ptr<statement> ret;
	SASL_TYPED_ATTRIBUTE( selector_attribute, typed_attr, attr );
	
	SASL_SWITCH_RULE( typed_attr->attr )
		SASL_CASE_RULE( stmt_flowctrl ){
			return build_flowctrl( typed_attr->attr );
		}
		SASL_CASE_RULE( stmt_decl ){
			return build_stmt_decl( typed_attr->attr );
		}
		SASL_CASE_RULE( stmt_expr ){
			return build_stmt_expr( typed_attr->attr );
		}
		SASL_CASE_RULE( stmt_if ){
			return build_stmt_if( typed_attr->attr );
		}
		SASL_CASE_RULE( stmt_compound ){
			return build_stmt_compound( typed_attr->attr );
		}
		SASL_CASE_RULE( stmt_for ){
			return build_stmt_for( typed_attr->attr );
		}
		SASL_CASE_RULE( stmt_while ){
			return build_stmt_while( typed_attr->attr );
		}
		SASL_CASE_RULE( stmt_dowhile ){
			return build_stmt_dowhile( typed_attr->attr );
		}
		SASL_CASE_RULE( stmt_switch ){
			return build_stmt_switch( typed_attr->attr );
		}
		SASL_CASE_RULE( labeled_stmt ){
			return build_stmt_labeled( typed_attr->attr );
		}
		SASL_DEFAULT(){
			string err;
			intptr_t rid = typed_attr->attr->rule_id();
			if( rid != -1 ){
				err = string( "Unprocessed rule: " );
				err += reinterpret_cast<sasl::parser::rule*>(rid)->name();
			}
			EFLIB_ASSERT_UNIMPLEMENTED0( err.c_str() );
			return ret;
		}
	SASL_END_SWITCH_RULE();

	return ret;
}

shared_ptr<compound_statement> syntax_tree_builder::build_stmt_compound( shared_ptr<attribute> attr ){
	shared_ptr<compound_statement> ret
		= create_node<compound_statement>( attr->token_beg(), attr->token_end() );
	SASL_TYPED_ATTRIBUTE( queuer_attribute, typed_attr, attr );
	SASL_TYPED_ATTRIBUTE( sequence_attribute, stmts_attr, typed_attr->attrs[1] );
	BOOST_FOREACH( shared_ptr<attribute> stmt_attr, stmts_attr->attrs ){
		ret->stmts.push_back( build_stmt(stmt_attr) );
	}
	return ret;
}

shared_ptr<jump_statement> syntax_tree_builder::build_flowctrl( shared_ptr<attribute> attr ){
	shared_ptr<jump_statement> ret = create_node<jump_statement>( attr->token_beg(), attr->token_end() );

	SASL_TYPED_ATTRIBUTE( selector_attribute, typed_attr, attr );
	shared_ptr<attribute> stmt_attr = typed_attr->attr;

	SASL_SWITCH_RULE( stmt_attr )
		SASL_CASE_RULE( stmt_break ){
			ret->code = jump_mode::_break;
		}
		SASL_CASE_RULE( stmt_continue ){
			ret->code = jump_mode::_continue;
		}
		SASL_CASE_RULE( stmt_return ){
			ret->code = jump_mode::_return;
			SASL_TYPED_ATTRIBUTE( queuer_attribute, ret_expr_attr, stmt_attr );
			SASL_TYPED_ATTRIBUTE( sequence_attribute, optional_expr_attr, ret_expr_attr->attrs[1] );
			if( !optional_expr_attr->attrs.empty() ){
				ret->jump_expr = build_expr( optional_expr_attr->attrs[0] );
			}
		}
	SASL_END_SWITCH_RULE();

	return ret;
}

shared_ptr<expression_statement> syntax_tree_builder::build_stmt_expr( shared_ptr<attribute> attr ){
	shared_ptr<expression_statement> ret = create_node<expression_statement>( attr->token_beg(), attr->token_end() );
	ret->expr = build_expr( attr->child(0) );
	return ret;
}

shared_ptr<declaration_statement> syntax_tree_builder::build_stmt_decl( shared_ptr<attribute> attr ){
	shared_ptr<declaration_statement> ret = create_node<declaration_statement>( attr->token_beg(), attr->token_end() );
	ret->decls = build_basic_decl( attr );
	return ret;
}

shared_ptr<if_statement> syntax_tree_builder::build_stmt_if( shared_ptr<attribute> attr )
{
	shared_ptr<if_statement> ret;
	shared_ptr<expression> expr = build_expr( attr->child(2) );
	shared_ptr<statement> yes_stmt = build_stmt( attr->child(4) );
	shared_ptr<statement> no_stmt;

	SASL_TYPED_ATTRIBUTE( sequence_attribute, optional_else_stmt, attr->child(5) );
	if( !optional_else_stmt->attrs.empty() ){
		no_stmt = build_stmt( optional_else_stmt->attrs[0]->child(1) );
		assert( no_stmt );
	}

	if( expr && yes_stmt ){
		ret = create_node<if_statement>( attr->token_beg(), attr->token_end() );
		ret->cond = expr;
		ret->yes_stmt = yes_stmt;
		ret->no_stmt = no_stmt;
	} else {
		assert(false);
	}

	return ret;
}

shared_ptr<for_statement> syntax_tree_builder::build_stmt_for( shared_ptr<attribute> attr )
{
	shared_ptr<for_statement> ret = build_for_loop( attr->child(1) ) ;

	if( ret ){
		shared_ptr<statement> body_stmt = build_stmt( attr->child(2) );
		ret->body = wrap_to_compound( body_stmt );
	} else {
		EFLIB_ASSERT_UNIMPLEMENTED();
	}
	
	return ret;
}

shared_ptr<while_statement> syntax_tree_builder::build_stmt_while( shared_ptr<attribute> attr )
{
	shared_ptr<expression> cond = build_expr( attr->child(2) );
	shared_ptr<statement> stmt = wrap_to_compound( build_stmt( attr->child(4) ) );
	assert( cond && stmt );

	if( cond && stmt ){
		shared_ptr<while_statement> ret = create_node<while_statement>( attr->token_beg(), attr->token_end() );
		ret->cond = cond;
		ret->body = stmt;
		return ret;
	}

	return shared_ptr<while_statement>();
}

shared_ptr<dowhile_statement> syntax_tree_builder::build_stmt_dowhile( shared_ptr<attribute> attr )
{
	shared_ptr<statement> stmt = wrap_to_compound( build_stmt( attr->child(1) ) );
	shared_ptr<expression> cond = build_expr( attr->child(4) );
	assert( cond && stmt );

	if( cond && stmt ){
		shared_ptr<dowhile_statement> ret = create_node<dowhile_statement>( attr->token_beg(), attr->token_end() );
		ret->cond = cond;
		ret->body = stmt;
		return ret;
	}

	return shared_ptr<dowhile_statement>();
}

shared_ptr<switch_statement> syntax_tree_builder::build_stmt_switch( shared_ptr<attribute> attr )
{
	shared_ptr<expression> cond = build_expr( attr->child(2) );
	shared_ptr<compound_statement> stmts = build_stmt_compound( attr->child(4) );

	assert( cond && stmts );

	shared_ptr<switch_statement> ret = create_node<switch_statement>( attr->token_beg(), attr->token_end() );
	ret->cond = cond;
	ret->stmts = stmts;
	return ret;
}

shared_ptr<statement> syntax_tree_builder::build_stmt_labeled( shared_ptr<attribute> attr )
{
	shared_ptr<label> lbl = build_label( attr->child(0) );
	shared_ptr<statement> stmt = build_stmt( attr->child(2) );
	assert( lbl && stmt );
	shared_ptr<labeled_statement> ret;
	if( stmt->node_class() == node_ids::labeled_statement ){
		ret = stmt->as_handle<labeled_statement>();
		ret->token_range( attr->token_beg(), attr->token_end() );
	} else {
		ret = create_node<labeled_statement>( attr->token_beg(), attr->token_end() );
		ret->stmt = stmt;
	}

	ret->labels.push_back( lbl );
	
	return ret;
}

shared_ptr<label> syntax_tree_builder::build_label( shared_ptr<attribute> attr )
{
	shared_ptr<attribute> label_attr = attr->child(0);

	SASL_SWITCH_RULE( label_attr )
		SASL_CASE_RULE( kw_default ){
			return create_node<case_label>( attr->token_beg(), attr->token_end() );
		}
		SASL_CASE_RULE( ident ){
			shared_ptr<ident_label> ret = create_node<ident_label>( attr->token_beg(), attr->token_end() );
			SASL_TYPED_ATTRIBUTE( terminal_attribute, ident_attr, label_attr );
			ret->label_tok = ident_attr->tok;
			return ret;
		}
		SASL_DEFAULT(){
			// Case Label
			assert( label_attr->child(0)->rule_id() == g.kw_case.id() );
			shared_ptr<case_label> ret = create_node<case_label>( attr->token_beg(), attr->token_end() );
			ret->expr = build_expr( label_attr->child(1) );
			return ret;
		}
	SASL_END_SWITCH_RULE();

	return shared_ptr<label>();
}

shared_ptr<for_statement> syntax_tree_builder::build_for_loop( shared_ptr<attribute> attr )
{
	shared_ptr<for_statement> ret = create_node<for_statement>( attr->token_beg(), attr->token_end() );

	ret->init = build_stmt( attr->child(1) );
	if( ret->init ){
		ret->cond = build_expr( attr->child(2)->child(0) );
		ret->iter = build_expr( attr->child(4)->child(0) );
	} else {
		ret.reset();
	}

	return ret;
}

shared_ptr<compound_statement> syntax_tree_builder::wrap_to_compound( shared_ptr<statement> stmt )
{
	if( stmt->node_class() == node_ids::compound_statement ){
		return stmt->as_handle<compound_statement>();
	}
	shared_ptr<compound_statement> ret_stmt = create_node<compound_statement>( stmt->token_begin(), stmt->token_end() );
	ret_stmt->stmts.push_back( stmt );
	return ret_stmt;
}

shared_ptr<tynode> syntax_tree_builder::bind_typequal( shared_ptr<tynode> unqual, shared_ptr<attribute> qual ){
	EFLIB_ASSERT_UNIMPLEMENTED();
	return unqual;
}

shared_ptr<tynode> syntax_tree_builder::bind_typequal( shared_ptr<attribute> qual, shared_ptr<tynode> unqual ){
	EFLIB_ASSERT_UNIMPLEMENTED();
	return unqual;
}

void syntax_tree_builder::build_initdecl(
	shared_ptr<attribute> attr,
	shared_ptr<tynode> tyn,
	vector< shared_ptr<declarator> >& declarators,
	vector< shared_ptr<variable_declaration> >& declarations )
{
	shared_ptr<declarator> decltor = create_node<declarator>( attr->token_beg(), attr->token_end() ) ;

	SASL_TYPED_ATTRIBUTE( queuer_attribute, typed_attr, attr );

	SASL_TYPED_ATTRIBUTE( terminal_attribute, name_attr, typed_attr->attrs[0] );
	decltor->name = name_attr->tok;
	
	shared_ptr<attribute> arr_qual_attr = typed_attr->child(1);
	if( arr_qual_attr->child_size() > 0 )
	{
		shared_ptr<array_type> arr_ty;

		if( tyn->is_array() ) {
			arr_ty = tyn->as_handle<array_type>();
		} else {
			arr_ty = create_node<array_type>( arr_qual_attr->token_beg(), arr_qual_attr->token_end() );
			arr_ty->elem_type = tyn;
		}

		vector< shared_ptr<expression> > arr_len_exprs;
		for( size_t i_dim = 0; i_dim < arr_qual_attr->child_size(); ++i_dim )
		{
			shared_ptr<attribute> dim_expr_attr = arr_qual_attr->child(i_dim)->child(1)->child(0);
			arr_len_exprs.push_back( dim_expr_attr ? build_expr(dim_expr_attr) : shared_ptr<expression>() );
		}
		arr_ty->array_lens.insert( arr_ty->array_lens.end(), arr_len_exprs.begin(), arr_len_exprs.end() );

		shared_ptr<variable_declaration> decl
			= create_node<variable_declaration>( attr->token_beg(), attr->token_end() );
		decl->type_info = arr_ty;
		decl->declarators.push_back(decltor);

		declarations.push_back(decl);
	}
	else
	{
		declarators.push_back(decltor);
	}

	build_semantic( typed_attr->attrs[2], decltor->semantic, decltor->semantic_index );
	
	SASL_TYPED_ATTRIBUTE( sequence_attribute, optional_anno_attr, typed_attr->attrs[3] );
	if( !optional_anno_attr->attrs.empty() ){
		EFLIB_ASSERT_UNIMPLEMENTED();
	}

	SASL_TYPED_ATTRIBUTE( sequence_attribute, optional_init_attr, typed_attr->attrs[4] );
	if( !optional_init_attr->attrs.empty() ){
		decltor->init = build_init( optional_init_attr->child(0) );
	}
}

operators syntax_tree_builder::build_binop(
	shared_ptr<attribute> attr,
	shared_ptr<token_t>& op_tok)
{
	// Get terminal attribute of operator from attr or direct child of attr.
	SASL_DYNCAST_ATTRIBUTE(terminal_attribute, tok_attr, attr);
	if( !tok_attr ){
		tok_attr = shared_polymorphic_cast<terminal_attribute>( attr->child(0) );
	}

	assert( tok_attr );
	op_tok = tok_attr->tok;
	std::string const& op_str = op_tok->str;
	char op_chars[4] = {'\0', '\0', '\0', '\0'};
	for( size_t i = 0; i < op_str.length(); ++i ){ op_chars[i] = op_str[i]; }

	switch( op_chars[0] ){
	case '=':
		return op_chars[1] == '=' ? operators::equal : operators::assign;
	case '+':
		return op_chars[1] == '=' ? operators::add_assign : operators::add;
	case '-':
		return op_chars[1] == '=' ? operators::sub_assign : operators::sub;
	case '*':
		return op_chars[1] == '=' ? operators::mul_assign : operators::mul;
	case '/':
		return op_chars[1] == '=' ? operators::div_assign : operators::div;
	case '%':
		return op_chars[1] == '=' ? operators::mod_assign : operators::mod;
	case '<':
		if ( op_chars[1] == '\0' ) return operators::less;
		if ( op_chars[1] == '<'  )
		{
			return op_chars[2] == '=' ? operators::lshift_assign : operators::left_shift;
		}
		if ( op_chars[1] == '='  ) return operators::less_equal;
		break;
	case '>':
		if ( op_chars[1] == '\0' ) return operators::greater;
		if ( op_chars[1] == '>'  )
		{
			return op_chars[2] == '=' ? operators::rshift_assign : operators::right_shift;
		}
		if ( op_chars[1] == '='  ) return operators::greater_equal;
		break;
	case '!':
		if ( op_chars[1] == '=' ) return operators::not_equal;
		break;
	case '|':
		if ( op_chars[1] == '|' ) return operators::logic_or;
		if ( op_chars[1] == '=' ) return operators::bit_or_assign;
		if ( op_chars[1] == '\0' ) return operators::bit_or;
		break;
	case '&':
		if ( op_chars[1] == '&' ) return operators::logic_and;
		if ( op_chars[1] == '=' ) return operators::bit_and_assign;
		if ( op_chars[1] == '\0' ) return operators::bit_and;
		break;
	case '^':
		if( op_chars[1] == '=' ) return operators::bit_xor_assign;
		if( op_chars[1] == '\0') return operators::bit_xor;
		break;
	}

	string assertion("Unsupported operator: ");
	assertion += op_str;
	EFLIB_ASSERT_UNIMPLEMENTED0( assertion.c_str() );

	return operators::none;
}

void syntax_tree_builder::build_semantic(
	shared_ptr<attribute> const& attr,
	shared_ptr<token_t>& out_semantic, shared_ptr<token_t>& out_semantic_index
	)
{
	SASL_TYPED_ATTRIBUTE( sequence_attribute, typed_attr, attr );
	if( !typed_attr->attrs.empty() ){
		SASL_TYPED_ATTRIBUTE( queuer_attribute, sem_attr, typed_attr->attrs[0] );
		SASL_TYPED_ATTRIBUTE( terminal_attribute, sem_name_attr, sem_attr->attrs[1] );
		out_semantic = sem_name_attr->tok;
		SASL_TYPED_ATTRIBUTE( sequence_attribute, optional_semindex_attr, sem_attr->attrs[2] );
		if( !optional_semindex_attr->attrs.empty() ){
			SASL_TYPED_ATTRIBUTE( queuer_attribute, parened_semindex_attr, optional_semindex_attr->attrs[0] );
			SASL_TYPED_ATTRIBUTE( terminal_attribute, index_attr, parened_semindex_attr->attrs[1] );
			out_semantic_index = index_attr->tok;
		}
	}
}

shared_ptr<builtin_type> syntax_tree_builder::get_builtin( boost::shared_ptr<sasl::parser::attribute> const& attr )
{
	initialize_bt_cache();
	SASL_TYPED_ATTRIBUTE( terminal_attribute, term_attr, attr );
	std::string name = term_attr->tok->str;
	shared_ptr<builtin_type> ret;
	if( bt_cache.count(name) > 0 )
	{
		ret = duplicate( bt_cache[name] )->as_handle<builtin_type>();
		ret->token_range( term_attr->tok, term_attr->tok );
	}
	return ret;
}

END_NS_SASL_SYNTAX_TREE()