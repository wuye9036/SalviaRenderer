#include <sasl/include/syntax_tree/syntax_tree_builder.h>

#include <sasl/include/parser/lexer.h>
#include <sasl/include/parser/generator.h>
#include <sasl/include/parser/grammars.h>

#include <sasl/enums/enums_helper.h>

#include <eflib/include/diagnostics/assert.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/foreach.hpp>
#include <eflib/include/platform/boost_end.h>

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
//
//expression_variant_visitor::expression_variant_visitor( syntax_tree_builder* builder, ast_builder_context& ctxt )
//: base_visitor(builder, ctxt){
//}
//
//shared_ptr<expression> expression_variant_visitor::operator()( const int& /*v*/ )
//{
//	assert(false);
//	return shared_ptr<expression>();
//}
//
//shared_ptr<expression> expression_variant_visitor::operator()( const constant_t& /*v*/ )
//{
//	EFLIB_ASSERT_UNIMPLEMENTED();
//	return shared_ptr<expression>();
//}
//
//template <typename ExpressionT>
//shared_ptr<expression> expression_variant_visitor::operator()( const ExpressionT& v )
//{
//	return builder->build( v, ctxt );
//}
//
//postfix_qualified_expression_visitor::postfix_qualified_expression_visitor(
//	syntax_tree_builder* builder, ast_builder_context& ctxt )
//	:base_visitor(builder, ctxt)
//{
//}
//
//shared_ptr<expression> postfix_qualified_expression_visitor::operator()( const sasl::parser_tree::idx_expression& v )
//{
//	return composite<index_expression>(v);
//}
//
//shared_ptr<expression> postfix_qualified_expression_visitor::operator()( const sasl::parser_tree::call_expression& v )
//{
//	return composite<call_expression>(v);
//}
//
//shared_ptr<expression> postfix_qualified_expression_visitor::operator()( const sasl::parser_tree::mem_expression& v )
//{
//	return composite<member_expression>(v);
//}
//
//shared_ptr<expression> postfix_qualified_expression_visitor::operator()( const token& v )
//{
//	return 
//		dexpr_combinator(NULL)
//			.dunary( builder->build_operator(v, operators::none) )
//			.dexpr().dnode(expr).end()
//		.end().typed_node2<expression>();
//}
//
//shared_ptr<statement> statement_visitor::operator()( const ::sasl::parser_tree::declaration_statement& v )
//{
//	shared_ptr<declaration_statement> ret;
//	shared_ptr<declaration> decl = builder->build(v, ctxt);
//	if( decl ){
//		ret = create_node<declaration_statement>( token::null() );
//		ret->decl = decl;
//	}
//	return ret;
//}
//
//
//template< typename STPostExpressionT, typename ExpressionPostT  >
//shared_ptr<expression> postfix_qualified_expression_visitor::composite( const ExpressionPostT& v )
//{
//	shared_ptr<expression> parent_node = builder->build(v, ctxt);
//	parent_node->typed_handle<STPostExpressionT>()->expr = expr;
//	return parent_node;
//}
//
//operators syntax_tree_builder::build_operator( const sasl::parser_tree::token& /*v*/, operators /*modifier*/ )
//{
//	EFLIB_ASSERT_UNIMPLEMENTED();
//	return operators::none;
//}
//
//shared_ptr<constant_expression> syntax_tree_builder::build_constant( const token& /*v*/ )
//{
//	EFLIB_ASSERT_UNIMPLEMENTED();
//	return shared_ptr<constant_expression>();
//}
//
//shared_ptr<expression> syntax_tree_builder::build( const sasl::parser_tree::unary_expression& v, ast_builder_context& ctxt )
//{
//	return build_variant_expression(v, ctxt);
//}
//
//shared_ptr<expression> syntax_tree_builder::build( const sasl::parser_tree::cast_expression& v, ast_builder_context& ctxt )
//{
//	return build_variant_expression(v, ctxt);
//}
//
//shared_ptr<expression> syntax_tree_builder::build( const sasl::parser_tree::mul_expression& v, ast_builder_context& ctxt )
//{
//	return build_binary_expression(v, ctxt);
//}
//
//shared_ptr<expression> syntax_tree_builder::build( const sasl::parser_tree::add_expression& v, ast_builder_context& ctxt )
//{
//	return build_binary_expression(v, ctxt);
//}
//
//shared_ptr<expression> syntax_tree_builder::build( const sasl::parser_tree::shf_expression& v, ast_builder_context& ctxt )
//{
//	return build_binary_expression(v, ctxt);
//}
//
//shared_ptr<expression> syntax_tree_builder::build( const sasl::parser_tree::rel_expression& v, ast_builder_context& ctxt )
//{
//	return build_binary_expression(v, ctxt);
//}
//
//shared_ptr<expression> syntax_tree_builder::build( const sasl::parser_tree::eql_expression& v, ast_builder_context& ctxt )
//{
//	return build_binary_expression(v, ctxt);
//}
//
//shared_ptr<expression> syntax_tree_builder::build( const sasl::parser_tree::band_expression& v, ast_builder_context& ctxt )
//{
//	return build_binary_expression(v, ctxt);
//}
//
//shared_ptr<expression> syntax_tree_builder::build( const sasl::parser_tree::bxor_expression& v, ast_builder_context& ctxt )
//{
//	return build_binary_expression(v, ctxt);
//}
//
//shared_ptr<expression> syntax_tree_builder::build( const sasl::parser_tree::bor_expression& v, ast_builder_context& ctxt )
//{
//	return build_binary_expression(v, ctxt);
//}
//
//shared_ptr<expression> syntax_tree_builder::build( const sasl::parser_tree::land_expression& v, ast_builder_context& ctxt )
//{
//	return build_binary_expression(v, ctxt);
//}
//
//shared_ptr<expression> syntax_tree_builder::build( const sasl::parser_tree::lor_expression& v, ast_builder_context& ctxt )
//{
//	return build_binary_expression(v, ctxt);
//}
//
//shared_ptr<expression> syntax_tree_builder::build( const sasl::parser_tree::rhs_expression& v, ast_builder_context& ctxt )
//{
//	return build_variant_expression(v, ctxt);
//}
//
//shared_ptr<expression> syntax_tree_builder::build( const sasl::parser_tree::assign_expression& v, ast_builder_context& ctxt )
//{
//	return build_right_combine_binary_expression( v, ctxt );
//}
//
//shared_ptr<expression> syntax_tree_builder::build( const sasl::parser_tree::expression& v, ast_builder_context& ctxt )
//{
//	if( v.follow_exprs.empty() ){
//		return build( v.first_expr, ctxt );
//	}
//
//	shared_ptr<expression_list> ret = create_node<expression_list>( token::null() );
//	ret->exprs.push_back( build(v.first_expr, ctxt) );
//
//	for( sasl::parser_tree::expression_lst::expr_list_t::const_iterator it = v.follow_exprs.begin(); it != v.follow_exprs.end(); ++it ){
//		shared_ptr<expression> expr = build( boost::fusion::at_c<1>(*it), ctxt );
//		ret->exprs.push_back( expr );
//	}
//	return ret;
//}
//
//shared_ptr<expression> syntax_tree_builder::build( const sasl::parser_tree::expression_post& v, ast_builder_context& ctxt )
//{
//	return build_variant_expression(v, ctxt);
//}
//
//shared_ptr<expression> syntax_tree_builder::build( const sasl::parser_tree::pm_expression& v, ast_builder_context& ctxt )
//{
//	expression_variant_visitor visitor( this, ctxt );
//	return boost::apply_visitor( visitor, v );
//}
//
//shared_ptr<expression> syntax_tree_builder::build( const sasl::parser_tree::typecast_expression& /*v*/, ast_builder_context& /*ctxt*/ )
//{
//	EFLIB_ASSERT_UNIMPLEMENTED();
//	return shared_ptr<expression>();
//}
//
//shared_ptr<expression> syntax_tree_builder::build( const sasl::parser_tree::post_expression& /*v*/, ast_builder_context& /*ctxt*/ )
//{
//	//shared_ptr<expression> expr = build(v.expr, ctxt);
//	//for(size_t i = 0; i < v.post.size(); ++i){
//	//	expr = append_expression_post( expr, v.post[i] );
//	//}
//	return shared_ptr<expression>(); //expr;
//}
//
//shared_ptr<expression> syntax_tree_builder::build( const sasl::parser_tree::cond_expression& v, ast_builder_context& ctxt )
//{
//	shared_ptr<cond_expression> ret = create_node<cond_expression>( token::null() );
//	ret->cond_expr = build( v.condexpr, ctxt );
//	ret->yes_expr = build( boost::fusion::at_c<1>(v.branchexprs), ctxt );
//	ret->no_expr = build( boost::fusion::at_c<3>(v.branchexprs), ctxt );
//	return ret;
//}
//
//shared_ptr<expression> syntax_tree_builder::build( const sasl::parser_tree::idx_expression& v, ast_builder_context& ctxt )
//{
//	shared_ptr<index_expression> ret = create_node<index_expression>( token::null() );
//	ret->index_expr = build(v.expr, ctxt);
//	return shared_ptr<expression>( ret );
//}
//
//shared_ptr<expression> syntax_tree_builder::build( const sasl::parser_tree::call_expression& v, ast_builder_context& ctxt )
//{
//	shared_ptr<call_expression> ret = create_node<call_expression>( token::null() );
//	if( v.args ){
//		shared_ptr<expression> arg_exprs = build( *(v.args), ctxt );
//		ret->args = arg_exprs->typed_handle<expression_list>()->exprs;
//	}
//	return ret;
//}
//
//shared_ptr<expression> syntax_tree_builder::build( const sasl::parser_tree::mem_expression& v, ast_builder_context& /*ctxt*/ )
//{
//	shared_ptr<member_expression> ret = create_node<member_expression>( token::null() );
//	ret->member = v.ident.make_copy();
//	return ret;
//}
//
//shared_ptr<expression> syntax_tree_builder::build( const sasl::parser_tree::unaried_expression& v, ast_builder_context& ctxt )
//{
//	shared_ptr<unary_expression> ret = create_node<unary_expression>( token::null() );
//	ret->expr = build( v.expr, ctxt );
//	ret->op = build_operator( v.preop, operators::none );
//	return shared_ptr<expression>(ret);
//}
//
//shared_ptr<expression> syntax_tree_builder::build( const sasl::parser_tree::paren_expression& v, ast_builder_context& ctxt )
//{
//	return build( v.expr, ctxt );
//}
//
//shared_ptr<statement> syntax_tree_builder::build( const sasl::parser_tree::statement& v, ast_builder_context& ctxt )
//{
//	statement_visitor visitor(this, ctxt);
//	return boost::apply_visitor(visitor, v);
//}
//
//shared_ptr<statement> syntax_tree_builder::build( const sasl::parser_tree::if_statement& v, ast_builder_context& ctxt )
//{
//	shared_ptr<if_statement> ret = create_node<if_statement>(v.if_keyword.make_copy());
//	ret->cond = build(v.cond, ctxt);
//	ret->yes_stmt = build(v.stmt, ctxt);
//	if (v.else_part){
//		ret->no_stmt = build( boost::fusion::at_c<1>(*(v.else_part)), ctxt );
//	}
//	return ret;
//}
//
//shared_ptr<program> syntax_tree_builder::build( const sasl::parser_tree::program& v )
//{
//	
//	shared_ptr<program> prog = create_node<program>( std::string("test") );
//	
//	ast_builder_context ctxt;
//	ctxt.parent = prog;
//
//	BOOST_FOREACH( const sasl::parser_tree::declaration& pt_decl, v ){
//		shared_ptr<declaration> gen_decl = build(pt_decl, ctxt);
//		if( gen_decl ){
//			// if variable declaration with multiple declarator,
//			// it will push itself into parent container and return null.
//			prog->decls.push_back( gen_decl );
//		}
//	}
//	return prog;
//}
//
//shared_ptr<declaration> syntax_tree_builder::build( const sasl::parser_tree::declaration& v, ast_builder_context& ctxt )
//{
//	dispatch_visitor< shared_ptr<declaration> > visitor(this, ctxt);
//	return boost::apply_visitor( visitor, v );
//}
//
//shared_ptr<declaration> syntax_tree_builder::build( const sasl::parser_tree::function_definition& /*v*/, ast_builder_context& /*ctxt*/ )
//{
//	EFLIB_ASSERT_UNIMPLEMENTED();
//	return shared_ptr<declaration>();
//}
//
//shared_ptr<declaration> syntax_tree_builder::build( const sasl::parser_tree::basic_declaration& v, ast_builder_context& ctxt )
//{
//	if (v.decl_body){
//		dispatch_visitor< shared_ptr<declaration> > visitor(this, ctxt);
//		return boost::apply_visitor( visitor, *v.decl_body );
//	}
//	return shared_ptr<declaration>();
//}
//
//shared_ptr<declaration> syntax_tree_builder::build( const sasl::parser_tree::variable_declaration& v, ast_builder_context& ctxt )
//{
//	shared_ptr<variable_declaration> ret = create_node<variable_declaration>( token::null() );
//	ret->type_info = build( v.declspec, ctxt );
//	
//	// build declarators
//	ret->declarators.push_back( build( v.decllist.first, ctxt ) );
//	typedef boost::fusion::vector< token, sasl::parser_tree::initialized_declarator > following_pair_t;
//	BOOST_FOREACH( following_pair_t const & follow_pair, v.decllist.follows	)
//	{
//		ret->declarators.push_back( build( boost::fusion::at_c<1>( follow_pair), ctxt ) );
//	}
//
//	return ret;
//}
//
//shared_ptr<declaration> syntax_tree_builder::build( const sasl::parser_tree::function_declaration& /*v*/, ast_builder_context& /*ctxt*/ )
//{
//	EFLIB_ASSERT_UNIMPLEMENTED();
//	return shared_ptr<declaration>();
//}
//
//shared_ptr<declaration> syntax_tree_builder::build( const sasl::parser_tree::typedef_declaration& /*v*/, ast_builder_context& /*ctxt*/ )
//{
//	EFLIB_ASSERT_UNIMPLEMENTED();
//	return shared_ptr<declaration>();
//}
//
//shared_ptr<declaration> syntax_tree_builder::build( const sasl::parser_tree::struct_declaration& /*v*/, ast_builder_context& /*ctxt*/ )
//{
//	EFLIB_ASSERT_UNIMPLEMENTED();
//	return shared_ptr<declaration>();
//}
//
//shared_ptr<type_specifier> syntax_tree_builder::build( const sasl::parser_tree::declaration_specifier& v, ast_builder_context& ctxt )
//{
//	shared_ptr<type_specifier> inner = build( v.unqual_type, ctxt );
//	return qualify( v.quals, inner );
//}
//
//shared_ptr<declarator> syntax_tree_builder::build( const sasl::parser_tree::initialized_declarator& v, ast_builder_context& ctxt )
//{
//	shared_ptr<declarator> ret = create_node<declarator>( token::null() );
//	if( !v.ident.empty() ){
//		ret->name = v.ident.make_copy();
//	}
//	if( v.init ){
//		ret->init = build( *(v.init), ctxt );
//	}
//	return ret;
//}
//
//shared_ptr<initializer> syntax_tree_builder::build( sasl::parser_tree::initializer const & v, ast_builder_context& ctxt ){
//	EFLIB_ASSERT_UNIMPLEMENTED();
//	return shared_ptr<initializer>();
//}
//
//boost::shared_ptr<type_specifier> syntax_tree_builder::build( sasl::parser_tree::prefix_qualified_type const & v, ast_builder_context& ctxt )
//{
//	shared_ptr<type_specifier> inner = build( v.unqual_type, ctxt );
//	return qualify( v.quals, inner );
//}
//
//boost::shared_ptr<type_specifier> syntax_tree_builder::build( sasl::parser_tree::unqualified_type const & v, ast_builder_context& ctxt )
//{
//	type_visitor visitor( this, ctxt );
//	return boost::apply_visitor( visitor, v );
//}
//
//boost::shared_ptr<type_specifier> syntax_tree_builder::build( sasl::parser_tree::paren_post_qualified_type const & v, ast_builder_context& ctxt )
//{
//	EFLIB_ASSERT_UNIMPLEMENTED();
//	return shared_ptr<type_specifier>();
//}
//
//boost::shared_ptr<type_specifier> syntax_tree_builder::qualify( sasl::parser_tree::postfix_qualified_type::qualifiers_t const & quals, boost::shared_ptr<type_specifier> inner )
//{
//	if( quals.empty() ) return inner;
//
//	EFLIB_ASSERT_UNIMPLEMENTED();
//	return shared_ptr<type_specifier>();
//}
//
//boost::shared_ptr<type_specifier> syntax_tree_builder::qualify( sasl::parser_tree::prefix_qualified_type::qualifiers_t const & quals, boost::shared_ptr<type_specifier> inner )
//{
//	if( quals.empty() ) return inner;
//
//	EFLIB_ASSERT_UNIMPLEMENTED();
//	return shared_ptr<type_specifier>();
//}
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


#define INSERT_INTO_TYPEMAP( litname, enum_code ) \
	{	\
		shared_ptr<builtin_type> bt = create_node<builtin_type>( token_t::null() );	\
		bt->value_typecode = builtin_type_code::enum_code;	\
		typemap.insert( make_pair( std::string( #litname ), bt ) );	\
	}

#define INSERT_VECTOR_INTO_TYPEMAP( component_type, dim, enum_code ) \
	{	\
		shared_ptr<builtin_type> bt = create_node<builtin_type>( token_t::null() );	\
		bt->value_typecode = sasl_ehelper::vector_of( builtin_type_code::enum_code, dim );	\
		typemap.insert( make_pair( string( #component_type ) + char_tbl[dim], bt ) );	\
	}

#define INSERT_MATRIX_INTO_TYPEMAP( component_type, dim0, dim1, enum_code ) \
	{	\
		shared_ptr<builtin_type> bt = create_node<builtin_type>( token_t::null() );	\
		bt->value_typecode = sasl_ehelper::matrix_of( builtin_type_code::enum_code, dim0, dim1 );	\
		typemap.insert( make_pair( string( #component_type ) + char_tbl[dim0] + "x" + char_tbl[dim1], bt ) );	\
	}

namespace builder_details{
	unordered_map< std::string, shared_ptr<builtin_type> > typemap;
	void initialize_typemap(){
		if( typemap.empty() ){

			char const char_tbl[] = { '0', '1', '2', '3', '4' };

			INSERT_INTO_TYPEMAP( sbyte,    _sint8   );
			INSERT_INTO_TYPEMAP( int8_t,   _sint8   );
			INSERT_INTO_TYPEMAP( ubyte,    _uint8   );
			INSERT_INTO_TYPEMAP( uint8_t,  _uint8   );
			INSERT_INTO_TYPEMAP( short,    _sint16  );
			INSERT_INTO_TYPEMAP( int16_t,  _sint16  );
			INSERT_INTO_TYPEMAP( ushort,   _uint16  );
			INSERT_INTO_TYPEMAP( uint16_t, _uint16  );
			INSERT_INTO_TYPEMAP( int,      _sint32  );
			INSERT_INTO_TYPEMAP( int32_t,  _sint32  );
			INSERT_INTO_TYPEMAP( uint,     _uint32  );
			INSERT_INTO_TYPEMAP( uint32_t, _uint32  );
			INSERT_INTO_TYPEMAP( long,     _uint32  );
			INSERT_INTO_TYPEMAP( int64_t,  _sint64  );
			INSERT_INTO_TYPEMAP( ulong,    _sint64  );
			INSERT_INTO_TYPEMAP( uint64_t, _uint64  );

			INSERT_INTO_TYPEMAP( float,    _float   );
			INSERT_INTO_TYPEMAP( double,   _double  );
			INSERT_INTO_TYPEMAP( bool,     _boolean );
			INSERT_INTO_TYPEMAP( void,     _void    );

			for( int dim0 = 1; dim0 <= 4; ++dim0 ){
				INSERT_VECTOR_INTO_TYPEMAP( int, dim0, _sint32  );
				INSERT_VECTOR_INTO_TYPEMAP( long, dim0, _sint64  );
				INSERT_VECTOR_INTO_TYPEMAP( float, dim0, _float  );
				INSERT_VECTOR_INTO_TYPEMAP( double, dim0, _double  );

				for( int dim1 = 1; dim1 <= 4; ++dim1 ){
					INSERT_MATRIX_INTO_TYPEMAP( int, dim0, dim1, _sint32  );
					INSERT_MATRIX_INTO_TYPEMAP( long, dim0, dim1, _sint64  );
					INSERT_MATRIX_INTO_TYPEMAP( float, dim0, dim1, _float  );
					INSERT_MATRIX_INTO_TYPEMAP( double, dim0, dim1, _double  );
				}
			}
		}
	}
}
using builder_details::typemap;

syntax_tree_builder::syntax_tree_builder( lexer& l, grammars& g ): l(l), g(g){}

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
			EFLIB_ASSERT_UNIMPLEMENTED();
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
	SASL_TYPED_ATTRIBUTE( sequence_attribute, params_attr, paren_params_attr->attrs[1] );
	BOOST_FOREACH( shared_ptr<attribute> param_attr, params_attr->attrs ){
		ret->params.push_back( build_param(param_attr) );
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
		ret->init = builtinit( optional_init->attrs[0] );
	}

	return ret;
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

shared_ptr<expression> syntax_tree_builder::build_assignexpr( shared_ptr<attribute> attr ){
	SASL_TYPED_ATTRIBUTE( queuer_attribute, typed_attr, attr );
	shared_ptr<expression> root;
		
	shared_ptr<expression> rexpr = build_rhsexpr( typed_attr->attrs[0] );

	shared_ptr<binary_expression> binexpr;
	SASL_TYPED_ATTRIBUTE( sequence_attribute, follows, typed_attr->attrs[1] );
	BOOST_FOREACH( shared_ptr<attribute> follow_pair, follows->attrs ){
		SASL_TYPED_ATTRIBUTE( queuer_attribute, typed_follow_pair, follow_pair );

		shared_ptr<expression> rexpr = binexpr->left_expr;
		binexpr = create_node<binary_expression>( token_t::null() );
		binexpr->left_expr = build_rhsexpr(typed_follow_pair->attrs[1]);;
		binexpr->op = build_binop(typed_follow_pair->attrs[0]);
		binexpr->right_expr = rexpr;

		if( !root ){ root = binexpr; }
	}

	return root ? root : rexpr;
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
	EFLIB_ASSERT_UNIMPLEMENTED();
	return shared_ptr<expression>();
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
	SASL_TYPED_ATTRIBUTE( queuer_attribute, typed_attr, attr );

	shared_ptr<expression> ret = build_pmexpr( typed_attr->attrs[0] );

	SASL_TYPED_ATTRIBUTE( sequence_attribute, postfix_attrs, typed_attr->attrs[1] );
	BOOST_FOREACH( shared_ptr<attribute> postfix_attr, postfix_attrs->attrs ){
		EFLIB_ASSERT_UNIMPLEMENTED();
	}

	return ret;
}

shared_ptr<expression> syntax_tree_builder::build_pmexpr( shared_ptr<attribute> attr ){
	SASL_TYPED_ATTRIBUTE( selector_attribute, typed_attr, attr );
	SASL_SWITCH_RULE( typed_attr->attr )
		SASL_CASE_RULE( lit_const ){
			EFLIB_ASSERT_UNIMPLEMENTED();
			return shared_ptr<expression>();
		}
		SASL_CASE_RULE( ident ){
			SASL_TYPED_ATTRIBUTE( terminal_attribute, var_attr, typed_attr->attr );
			shared_ptr<variable_expression> varexpr = create_node<variable_expression>( var_attr->tok );
			varexpr->var_name = var_attr->tok;
			return varexpr;
		}
		SASL_CASE_RULE( parenexpr ){
			EFLIB_ASSERT_UNIMPLEMENTED();
			return shared_ptr<expression>();
		}
	SASL_END_SWITCH_RULE();

	return shared_ptr<expression>();
}

shared_ptr<cast_expression> syntax_tree_builder::build_typecastedexpr( shared_ptr<attribute> attr ){
	EFLIB_ASSERT_UNIMPLEMENTED();
	return shared_ptr<cast_expression>();
}

shared_ptr<type_specifier> syntax_tree_builder::build_typespec( shared_ptr<attribute> attr ){
	return build_postqualedtype( attr );
}

vector< shared_ptr<declarator> > syntax_tree_builder::build_declarators( shared_ptr<attribute> attr ){
	SASL_TYPED_ATTRIBUTE( queuer_attribute, typed_attr, attr );

	vector< shared_ptr<declarator> > ret;
	ret.push_back( builtinitdecl(typed_attr->attrs[0]) );

	SASL_TYPED_ATTRIBUTE( sequence_attribute, follows, typed_attr->attrs[1] );
	BOOST_FOREACH( shared_ptr<attribute> follow_attr, follows->attrs ){
		SASL_TYPED_ATTRIBUTE( queuer_attribute, follow_pair, follow_attr );
		ret.push_back( builtinitdecl(follow_pair->attrs[1]) );
	}

	return ret;
}

shared_ptr<type_specifier> syntax_tree_builder::build_unqualedtype( shared_ptr<attribute> attr ){
	shared_ptr<type_specifier> ret;

	SASL_TYPED_ATTRIBUTE( selector_attribute, typed_attr, attr );

	SASL_SWITCH_RULE( typed_attr->attr )
		SASL_CASE_RULE( ident ){
			SASL_TYPED_ATTRIBUTE( terminal_attribute, ident_attr, typed_attr->attr );
			builder_details::initialize_typemap();
			if ( typemap.count(ident_attr->tok->str) > 0 ){
				return typemap[ident_attr->tok->str];
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

shared_ptr<type_specifier> syntax_tree_builder::build_prequaledtype( shared_ptr<attribute> attr ){
	SASL_TYPED_ATTRIBUTE( queuer_attribute, typed_attr, attr );
	shared_ptr<type_specifier> ret_type = build_unqualedtype( typed_attr->attrs[1] );

	SASL_TYPED_ATTRIBUTE( sequence_attribute, quals_attr, typed_attr->attrs[0] );
	BOOST_FOREACH( shared_ptr<attribute> qual_attr, quals_attr->attrs ){
		ret_type = bind_typequal( attr, ret_type );
	}

	return ret_type;
}

shared_ptr<type_specifier> syntax_tree_builder::build_postqualedtype( shared_ptr<attribute> attr )
{
	SASL_TYPED_ATTRIBUTE( queuer_attribute, typed_attr, attr );
	shared_ptr<type_specifier> ret_type = build_prequaledtype( typed_attr->attrs[0] );

	SASL_TYPED_ATTRIBUTE( sequence_attribute, quals_attr, typed_attr->attrs[1] );
	BOOST_FOREACH( shared_ptr<attribute> qual_attr, quals_attr->attrs ){
		ret_type = bind_typequal( ret_type, attr );
	}

	return ret_type;
}

shared_ptr<initializer> syntax_tree_builder::builtinit( shared_ptr<attribute> attr ){
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

shared_ptr<type_specifier> syntax_tree_builder::bind_typequal( shared_ptr<type_specifier> unqual, shared_ptr<attribute> qual ){
	EFLIB_ASSERT_UNIMPLEMENTED();
	return unqual;
}

shared_ptr<type_specifier> syntax_tree_builder::bind_typequal( shared_ptr<attribute> qual, shared_ptr<type_specifier> unqual ){
	EFLIB_ASSERT_UNIMPLEMENTED();
	return unqual;
}

shared_ptr<declarator> syntax_tree_builder::builtinitdecl( shared_ptr<attribute> attr ){
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
		EFLIB_ASSERT_UNIMPLEMENTED();
	}

	return ret;
}

operators syntax_tree_builder::build_binop( boost::shared_ptr<sasl::parser::attribute> attr ){
	EFLIB_ASSERT_UNIMPLEMENTED();
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