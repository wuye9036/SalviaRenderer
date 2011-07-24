#include <sasl/include/syntax_tree/syntax_tree_builder.h>

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
		shared_ptr<builtin_type> bt = create_node<builtin_type>( token_t::null() );	\
		bt->value_typecode = builtin_types::enum_code;	\
		bt_cache.insert( make_pair( std::string( #litname ), bt ) );	\
	}

#define INSERT_VECTOR_INTO_BTCACHE( component_type, dim, enum_code ) \
	{	\
		shared_ptr<builtin_type> bt = create_node<builtin_type>( token_t::null() );	\
		bt->value_typecode = vector_of( builtin_types::enum_code, dim );	\
		bt_cache.insert( make_pair( string( #component_type ) + char_tbl[dim], bt ) );	\
	}

#define INSERT_MATRIX_INTO_BTCACHE( component_type, dim0, dim1, enum_code ) \
	{	\
		shared_ptr<builtin_type> bt = create_node<builtin_type>( token_t::null() );	\
		bt->value_typecode = matrix_of( builtin_types::enum_code, dim0, dim1 );	\
		bt_cache.insert( make_pair( string( #component_type ) + char_tbl[dim0] + "x" + char_tbl[dim1], bt ) );	\
	}

syntax_tree_builder::syntax_tree_builder( lexer& l, grammars& g ): l(l), g(g){}

void syntax_tree_builder::initialize_bt_cache(){
	if( bt_cache.empty() ){

		char const char_tbl[] = { '0', '1', '2', '3', '4' };

		INSERT_INTO_BTCACHE( sbyte,    _sint8   );
		INSERT_INTO_BTCACHE( int8_t,   _sint8   );
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
		INSERT_INTO_BTCACHE( long,     _uint32  );
		INSERT_INTO_BTCACHE( int64_t,  _sint64  );
		INSERT_INTO_BTCACHE( ulong,    _sint64  );
		INSERT_INTO_BTCACHE( uint64_t, _uint64  );

		INSERT_INTO_BTCACHE( float,    _float   );
		INSERT_INTO_BTCACHE( double,   _double  );
		INSERT_INTO_BTCACHE( bool,     _boolean );
		INSERT_INTO_BTCACHE( void,     _void    );

		for( int dim0 = 1; dim0 <= 4; ++dim0 ){
			INSERT_VECTOR_INTO_BTCACHE( int, dim0, _sint32  );
			INSERT_VECTOR_INTO_BTCACHE( long, dim0, _sint64  );
			INSERT_VECTOR_INTO_BTCACHE( float, dim0, _float  );
			INSERT_VECTOR_INTO_BTCACHE( double, dim0, _double  );

			for( int dim1 = 1; dim1 <= 4; ++dim1 ){
				INSERT_MATRIX_INTO_BTCACHE( int, dim0, dim1, _sint32  );
				INSERT_MATRIX_INTO_BTCACHE( long, dim0, dim1, _sint64  );
				INSERT_MATRIX_INTO_BTCACHE( float, dim0, dim1, _float  );
				INSERT_MATRIX_INTO_BTCACHE( double, dim0, dim1, _double  );
			}
		}
	}
}

shared_ptr<program> syntax_tree_builder::build_prog( shared_ptr< attribute > attr )
{
	shared_ptr<program> ret;
	
	SASL_TYPED_ATTRIBUTE(sequence_attribute, typed_attr, attr);

	if( typed_attr ){
		ret = create_node<program>("prog");

		BOOST_FOREACH( shared_ptr<attribute> decl_attr, typed_attr->attrs ){
			shared_ptr<declaration> decl = build_decl( decl_attr );
			if(decl){
				ret->decls.push_back(decl);
			}
		}
	}

	return ret;
}

boost::shared_ptr<declaration> syntax_tree_builder::build_decl( shared_ptr<attribute> attr )
{
	shared_ptr<declaration> ret;
	
	SASL_TYPED_ATTRIBUTE(selector_attribute, typed_attr, attr);
	EFLIB_ASSERT_AND_IF( typed_attr->selected_idx >= 0, "Attribute error: least one branch was selected." ){
		return ret;
	}
	
	SASL_SWITCH_RULE( typed_attr->attr )
		SASL_CASE_RULE( basic_decl ){
			return build_basic_decl( typed_attr->attr );
		}
		SASL_CASE_RULE( function_def ){
			return build_fndef( typed_attr->attr );
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

shared_ptr<declaration> syntax_tree_builder::build_basic_decl( shared_ptr<attribute> attr ){
	shared_ptr<declaration> ret;

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
			return build_vardecl(typed_decl_attr->attr);
		}
		SASL_CASE_RULE( function_decl ){
			EFLIB_ASSERT_UNIMPLEMENTED();
		}
		SASL_CASE_RULE( struct_decl ){
			return build_struct( typed_decl_attr->attr );
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

shared_ptr<variable_declaration> syntax_tree_builder::build_vardecl( shared_ptr<attribute> attr ){
	shared_ptr<variable_declaration> ret;

	SASL_TYPED_ATTRIBUTE( queuer_attribute, typed_attr, attr );

	assert( typed_attr->attrs.size() == 2 );

	ret = create_node<variable_declaration>( token_t::null() );
	ret->type_info = build_typespec( typed_attr->attrs[0] );
	ret->declarators = build_declarators( typed_attr->attrs[1] );
	
	return ret;
}

shared_ptr<function_type> syntax_tree_builder::build_fndecl( shared_ptr<attribute> attr ){
	shared_ptr<function_type> ret = create_node<function_type>( token_t::null() );

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
	shared_ptr<parameter> ret = create_node<parameter>( token_t::null() );
	
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
	shared_ptr<struct_type> ret = create_node<struct_type>( token_t::null() );

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

	BOOST_FOREACH( shared_ptr<attribute> const& decl_attr, decls_attr->attrs ){
		shared_ptr<declaration> decl = build_decl(decl_attr);
		if( decl ){
			out->decls.push_back( decl );
		}
	}
}

shared_ptr<expression> syntax_tree_builder::build_expr( shared_ptr<attribute> attr ){
	shared_ptr<expression_list> ret = build_exprlst( attr );
	if ( ret->exprs.size() == 1 ){
		return ret->exprs[0];
	}
	return ret;
}

shared_ptr<expression_list> syntax_tree_builder::build_exprlst( shared_ptr<attribute> attr ){
	shared_ptr<expression_list> ret = create_node<expression_list>( token_t::null() );

	SASL_TYPED_ATTRIBUTE( queuer_attribute, typed_attr, attr );
	ret->exprs.push_back( build_assignexpr( typed_attr->attrs[0] ) );

	SASL_TYPED_ATTRIBUTE( sequence_attribute, follows, typed_attr->attrs[1] );
	BOOST_FOREACH( shared_ptr<attribute> follow_pair, follows->attrs ){
		SASL_TYPED_ATTRIBUTE( queuer_attribute, typed_follow_pair, follow_pair );
		ret->exprs.push_back( build_assignexpr( typed_follow_pair->attrs[1] ) );
	}

	return ret;
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
\*/
shared_ptr<expression> syntax_tree_builder::build_assignexpr( shared_ptr<attribute> attr ){

	// Make expression list and operators list.
	vector< shared_ptr<expression> > exprs;
	vector< operators > ops;

	exprs.push_back( build_rhsexpr( attr->child(0) ) );
	SASL_TYPED_ATTRIBUTE( sequence_attribute, follows, attr->child(1) );
	BOOST_FOREACH( shared_ptr<attribute> follow_pair, follows->attrs ){
		exprs.push_back( 
			build_rhsexpr( follow_pair->child(1) )
			);
		ops.push_back( build_binop(follow_pair->child(0)) );
	}

	// Build tree
	shared_ptr<expression> root;

	BOOST_REVERSE_FOREACH( shared_ptr<expression> const& expr, exprs ){
		if( !root ){
			root = expr;
		} else {
			shared_ptr<binary_expression> new_root
				= create_node<binary_expression>( token_t::null() );
			new_root->left_expr = root;
			new_root->right_expr = expr;
			new_root->op = ops.back();
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
		SASL_TYPED_ATTRIBUTE( queuer_attribute, typed_follow_pair, follow_pair );
		binexpr = create_node<binary_expression>( token_t::null() );
		binexpr->left_expr = lexpr;
		binexpr->op = build_binop(typed_follow_pair->attrs[0]);
		binexpr->right_expr = dispatch_lcomb_expr(typed_follow_pair->attrs[1]);
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

	shared_ptr<cond_expression> ret = create_node<cond_expression>( token_t::null() );

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
	EFLIB_ASSERT_UNIMPLEMENTED();
	return shared_ptr<unary_expression>();
}

shared_ptr<expression> syntax_tree_builder::build_postexpr( shared_ptr<attribute> attr ){
	shared_ptr<expression> ret = build_pmexpr( attr->child(0) );

	SASL_TYPED_ATTRIBUTE( sequence_attribute, postfix_attrs, attr->child(1) );
	BOOST_FOREACH( shared_ptr<attribute> postfix_attr, postfix_attrs->attrs ){
		shared_ptr<attribute> expr_attr = postfix_attr->child(0);
		SASL_SWITCH_RULE( expr_attr )
			SASL_CASE_RULE( idxexpr ){
				EFLIB_ASSERT_UNIMPLEMENTED();
			}
			SASL_CASE_RULE( callexpr ){
				ret = build_callexpr( expr_attr->child(1), ret );
			}
			SASL_CASE_RULE( memexpr ){
				ret = build_memexpr(expr_attr, ret);
			}
			SASL_CASE_RULE( opinc ){
				EFLIB_ASSERT_UNIMPLEMENTED();
			}
		SASL_END_SWITCH_RULE();
	}

	return ret;
}

shared_ptr<expression> syntax_tree_builder::build_callexpr(
	shared_ptr<attribute> attr,
	shared_ptr<expression> expr )
{
	shared_ptr<call_expression> ret = create_node<call_expression>( token_t::null() );
	ret->expr = expr;

	SASL_TYPED_ATTRIBUTE( sequence_attribute, optional_args, attr );
	if( !optional_args->attrs.empty() )
	{
		shared_ptr<expression_list> arglst = build_exprlst( attr->child(0) );
		ret->args = arglst->exprs;
	}
	
	return ret;
}

shared_ptr<expression> syntax_tree_builder::build_memexpr(
	shared_ptr<attribute> attr,
	shared_ptr<expression> expr )
{
	shared_ptr<member_expression> ret = create_node<member_expression>( token_t::null() );
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
			shared_ptr<constant_expression> ret = create_node<constant_expression>( const_attr->tok );
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
			shared_ptr<variable_expression> varexpr = create_node<variable_expression>( var_attr->tok );
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

vector< shared_ptr<declarator> > syntax_tree_builder::build_declarators( shared_ptr<attribute> attr ){
	SASL_TYPED_ATTRIBUTE( queuer_attribute, typed_attr, attr );

	vector< shared_ptr<declarator> > ret;
	ret.push_back( build_initdecl(typed_attr->attrs[0]) );

	SASL_TYPED_ATTRIBUTE( sequence_attribute, follows, typed_attr->attrs[1] );
	BOOST_FOREACH( shared_ptr<attribute> follow_attr, follows->attrs ){
		SASL_TYPED_ATTRIBUTE( queuer_attribute, follow_pair, follow_attr );
		ret.push_back( build_initdecl(follow_pair->attrs[1]) );
	}

	return ret;
}

shared_ptr<tynode> syntax_tree_builder::build_unqualedtype( shared_ptr<attribute> attr ){
	shared_ptr<tynode> ret;

	SASL_TYPED_ATTRIBUTE( selector_attribute, typed_attr, attr );

	SASL_SWITCH_RULE( typed_attr->attr )
		SASL_CASE_RULE( ident ){
			SASL_TYPED_ATTRIBUTE( terminal_attribute, ident_attr, typed_attr->attr );
			initialize_bt_cache();
			if ( bt_cache.count(ident_attr->tok->str) > 0 ){
				return bt_cache[ident_attr->tok->str];
			}

			shared_ptr<alias_type> type_ident = create_node<alias_type>( token_t::null() );
			type_ident->alias = ident_attr->tok->make_copy();

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
	EFLIB_ASSERT_UNIMPLEMENTED();
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
		SASL_DEFAULT(){
			EFLIB_ASSERT_UNIMPLEMENTED();
			return ret;
		}
	SASL_END_SWITCH_RULE();

	return ret;
}

shared_ptr<compound_statement> syntax_tree_builder::build_stmt_compound( shared_ptr<attribute> attr ){
	shared_ptr<compound_statement> ret = create_node<compound_statement>( token_t::null() );
	SASL_TYPED_ATTRIBUTE( queuer_attribute, typed_attr, attr );
	SASL_TYPED_ATTRIBUTE( sequence_attribute, stmts_attr, typed_attr->attrs[1] );
	BOOST_FOREACH( shared_ptr<attribute> stmt_attr, stmts_attr->attrs ){
		ret->stmts.push_back( build_stmt(stmt_attr) );
	}
	return ret;
}

shared_ptr<jump_statement> syntax_tree_builder::build_flowctrl( shared_ptr<attribute> attr ){
	shared_ptr<jump_statement> ret = create_node<jump_statement>( token_t::null() );

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
	shared_ptr<expression_statement> ret = create_node<expression_statement>( token_t::null() );
	ret->expr = build_expr( attr->child(0) );
	return ret;
}

shared_ptr<declaration_statement> syntax_tree_builder::build_stmt_decl( shared_ptr<attribute> attr ){
	shared_ptr<declaration_statement> ret = create_node<declaration_statement>( token_t::null() );
	ret->decl = build_decl( attr );
	return ret;
}

shared_ptr<tynode> syntax_tree_builder::bind_typequal( shared_ptr<tynode> unqual, shared_ptr<attribute> qual ){
	EFLIB_ASSERT_UNIMPLEMENTED();
	return unqual;
}

shared_ptr<tynode> syntax_tree_builder::bind_typequal( shared_ptr<attribute> qual, shared_ptr<tynode> unqual ){
	EFLIB_ASSERT_UNIMPLEMENTED();
	return unqual;
}

shared_ptr<declarator> syntax_tree_builder::build_initdecl( shared_ptr<attribute> attr ){
	shared_ptr<declarator> ret = create_node<declarator>( token_t::null() ) ;

	SASL_TYPED_ATTRIBUTE( queuer_attribute, typed_attr, attr );

	SASL_TYPED_ATTRIBUTE( terminal_attribute, name_attr, typed_attr->attrs[0] );
	ret->name = name_attr->tok->make_copy();
	
	build_semantic( typed_attr->attrs[1], ret->semantic, ret->semantic_index );
	
	SASL_TYPED_ATTRIBUTE( sequence_attribute, optional_anno_attr, typed_attr->attrs[2] );
	if( !optional_anno_attr->attrs.empty() ){
		EFLIB_ASSERT_UNIMPLEMENTED();
	}

	SASL_TYPED_ATTRIBUTE( sequence_attribute, optional_init_attr, typed_attr->attrs[3] );
	if( !optional_init_attr->attrs.empty() ){
		ret->init = build_init( optional_init_attr->child(0) );
	}

	return ret;
}

operators syntax_tree_builder::build_binop( shared_ptr<attribute> attr ){
	SASL_DYNCAST_ATTRIBUTE( terminal_attribute, tok_attr, attr );
	if( !tok_attr ){
		tok_attr = shared_polymorphic_cast<terminal_attribute>( attr->child(0) );
	}

	assert( tok_attr );

	std::string const& op_str = tok_attr->tok->str;
	char op_chars[4] = {'\0', '\0', '\0', '\0'};
	for( size_t i = 0; i < op_str.length(); ++i ){ op_chars[i] = op_str[i]; }

	switch( op_chars[0] ){
	case '=':
		return op_chars[1] == '='? operators::equal : operators::assign;
	case '+':
		return op_chars[1] == '=' ? operators::add_assign : operators::add;
	case '-':
		return op_chars[1] == '=' ? operators::sub_assign : operators::sub;
	case '*':
		return op_chars[1] == '=' ? operators::mul_assign : operators::mul;
	case '/':
		return op_chars[1] == '=' ? operators::div_assign : operators::div;
	case '<':
		return op_chars[1] == '=' ? operators::less_equal : operators::less;
	case '>':
		return op_chars[1] == '=' ? operators::greater_equal : operators::greater;
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

END_NS_SASL_SYNTAX_TREE()