#include <sasl/include/syntax_tree/syntax_tree_builder.h>

#include <sasl/include/syntax_tree/make_tree.h>

#include <eflib/include/diagnostics/assert.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/foreach.hpp>
#include <eflib/include/platform/boost_end.h>

using boost::shared_ptr;
using sasl::common::token_attr;

BEGIN_NS_SASL_SYNTAX_TREE();

expression_variant_visitor::expression_variant_visitor( syntax_tree_builder* builder, ast_builder_context& ctxt )
: base_visitor(builder, ctxt){
}

shared_ptr<expression> expression_variant_visitor::operator()( const int& /*v*/ )
{
	assert(false);
	return shared_ptr<expression>();
}

shared_ptr<expression> expression_variant_visitor::operator()( const constant_t& /*v*/ )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return shared_ptr<expression>();
}

template <typename ExpressionT>
shared_ptr<expression> expression_variant_visitor::operator()( const ExpressionT& v )
{
	return builder->build( v, ctxt );
}

postfix_qualified_expression_visitor::postfix_qualified_expression_visitor(
	syntax_tree_builder* builder, ast_builder_context& ctxt )
	:base_visitor(builder, ctxt)
{
}

shared_ptr<expression> postfix_qualified_expression_visitor::operator()( const sasl::parser_tree::idx_expression& v )
{
	return composite<index_expression>(v);
}

shared_ptr<expression> postfix_qualified_expression_visitor::operator()( const sasl::parser_tree::call_expression& v )
{
	return composite<call_expression>(v);
}

shared_ptr<expression> postfix_qualified_expression_visitor::operator()( const sasl::parser_tree::mem_expression& v )
{
	return composite<member_expression>(v);
}

shared_ptr<expression> postfix_qualified_expression_visitor::operator()( const token_attr& v )
{
	return 
		dexpr_combinator(NULL)
			.dunary( builder->build_operator(v, operators::none) )
			.dexpr().dnode(expr).end()
		.end().typed_node2<expression>();
}

boost::shared_ptr<statement> statement_visitor::operator()( const ::sasl::parser_tree::declaration_statement& v )
{
	boost::shared_ptr<declaration_statement> ret;
	boost::shared_ptr<declaration> decl = builder->build(v, ctxt);
	if( decl ){
		ret = create_node<declaration_statement>( token_attr::null() );
		ret->decl = decl;
	}
	return ret;
}

template< typename STPostExpressionT, typename ExpressionPostT  >
shared_ptr<expression> postfix_qualified_expression_visitor::composite( const ExpressionPostT& v )
{
	shared_ptr<expression> parent_node = builder->build(v, ctxt);
	parent_node->typed_handle<STPostExpressionT>()->expr = expr;
	return parent_node;
}

operators syntax_tree_builder::build_operator( const sasl::parser_tree::token_attr& /*v*/, operators /*modifier*/ )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return operators::none;
}

shared_ptr<constant_expression> syntax_tree_builder::build_constant( const token_attr& /*v*/ )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return shared_ptr<constant_expression>();
}

shared_ptr<expression> syntax_tree_builder::build( const sasl::parser_tree::unary_expression& v, ast_builder_context& ctxt )
{
	return build_variant_expression(v, ctxt);
}

shared_ptr<expression> syntax_tree_builder::build( const sasl::parser_tree::cast_expression& v, ast_builder_context& ctxt )
{
	return build_variant_expression(v, ctxt);
}

shared_ptr<expression> syntax_tree_builder::build( const sasl::parser_tree::mul_expression& v, ast_builder_context& ctxt )
{
	return build_binary_expression(v, ctxt);
}

shared_ptr<expression> syntax_tree_builder::build( const sasl::parser_tree::add_expression& v, ast_builder_context& ctxt )
{
	return build_binary_expression(v, ctxt);
}

shared_ptr<expression> syntax_tree_builder::build( const sasl::parser_tree::shf_expression& v, ast_builder_context& ctxt )
{
	return build_binary_expression(v, ctxt);
}

shared_ptr<expression> syntax_tree_builder::build( const sasl::parser_tree::rel_expression& v, ast_builder_context& ctxt )
{
	return build_binary_expression(v, ctxt);
}

shared_ptr<expression> syntax_tree_builder::build( const sasl::parser_tree::eql_expression& v, ast_builder_context& ctxt )
{
	return build_binary_expression(v, ctxt);
}

shared_ptr<expression> syntax_tree_builder::build( const sasl::parser_tree::band_expression& v, ast_builder_context& ctxt )
{
	return build_binary_expression(v, ctxt);
}

shared_ptr<expression> syntax_tree_builder::build( const sasl::parser_tree::bxor_expression& v, ast_builder_context& ctxt )
{
	return build_binary_expression(v, ctxt);
}

shared_ptr<expression> syntax_tree_builder::build( const sasl::parser_tree::bor_expression& v, ast_builder_context& ctxt )
{
	return build_binary_expression(v, ctxt);
}

shared_ptr<expression> syntax_tree_builder::build( const sasl::parser_tree::land_expression& v, ast_builder_context& ctxt )
{
	return build_binary_expression(v, ctxt);
}

shared_ptr<expression> syntax_tree_builder::build( const sasl::parser_tree::lor_expression& v, ast_builder_context& ctxt )
{
	return build_binary_expression(v, ctxt);
}

shared_ptr<expression> syntax_tree_builder::build( const sasl::parser_tree::rhs_expression& v, ast_builder_context& ctxt )
{
	return build_variant_expression(v, ctxt);
}

shared_ptr<expression> syntax_tree_builder::build( const sasl::parser_tree::assign_expression& v, ast_builder_context& ctxt )
{
	return build_right_combine_binary_expression( v, ctxt );
}

shared_ptr<expression> syntax_tree_builder::build( const sasl::parser_tree::expression& v, ast_builder_context& ctxt )
{
	if( v.follow_exprs.empty() ){
		return build( v.first_expr, ctxt );
	}

	shared_ptr<expression_list> ret = create_node<expression_list>( token_attr::null() );
	ret->exprs.push_back( build(v.first_expr, ctxt) );

	for( sasl::parser_tree::expression_lst::expr_list_t::const_iterator it = v.follow_exprs.begin(); it != v.follow_exprs.end(); ++it ){
		shared_ptr<expression> expr = build( boost::fusion::at_c<1>(*it), ctxt );
		ret->exprs.push_back( expr );
	}
	return ret;
}

shared_ptr<expression> syntax_tree_builder::build( const sasl::parser_tree::expression_post& v, ast_builder_context& ctxt )
{
	return build_variant_expression(v, ctxt);
}

shared_ptr<expression> syntax_tree_builder::build( const sasl::parser_tree::pm_expression& v, ast_builder_context& ctxt )
{
	expression_variant_visitor visitor( this, ctxt );
	return boost::apply_visitor( visitor, v );
}

shared_ptr<expression> syntax_tree_builder::build( const sasl::parser_tree::typecast_expression& /*v*/, ast_builder_context& /*ctxt*/ )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return shared_ptr<expression>();
}

shared_ptr<expression> syntax_tree_builder::build( const sasl::parser_tree::post_expression& /*v*/, ast_builder_context& /*ctxt*/ )
{
	//shared_ptr<expression> expr = build(v.expr, ctxt);
	//for(size_t i = 0; i < v.post.size(); ++i){
	//	expr = append_expression_post( expr, v.post[i] );
	//}
	return shared_ptr<expression>(); //expr;
}

shared_ptr<expression> syntax_tree_builder::build( const sasl::parser_tree::cond_expression& v, ast_builder_context& ctxt )
{
	shared_ptr<cond_expression> ret = create_node<cond_expression>( token_attr::null() );
	ret->cond_expr = build( v.condexpr, ctxt );
	ret->yes_expr = build( boost::fusion::at_c<1>(v.branchexprs), ctxt );
	ret->no_expr = build( boost::fusion::at_c<3>(v.branchexprs), ctxt );
	return ret;
}

shared_ptr<expression> syntax_tree_builder::build( const sasl::parser_tree::idx_expression& v, ast_builder_context& ctxt )
{
	shared_ptr<index_expression> ret = create_node<index_expression>( token_attr::null() );
	ret->index_expr = build(v.expr, ctxt);
	return shared_ptr<expression>( ret );
}

shared_ptr<expression> syntax_tree_builder::build( const sasl::parser_tree::call_expression& v, ast_builder_context& ctxt )
{
	shared_ptr<call_expression> ret = create_node<call_expression>( token_attr::null() );
	if( v.args ){
		shared_ptr<expression> arg_exprs = build( *(v.args), ctxt );
		ret->args = arg_exprs->typed_handle<expression_list>()->exprs;
	}
	return ret;
}

shared_ptr<expression> syntax_tree_builder::build( const sasl::parser_tree::mem_expression& v, ast_builder_context& /*ctxt*/ )
{
	shared_ptr<member_expression> ret = create_node<member_expression>( token_attr::null() );
	ret->member = v.ident.make_copy();
	return ret;
}

shared_ptr<expression> syntax_tree_builder::build( const sasl::parser_tree::unaried_expression& v, ast_builder_context& ctxt )
{
	shared_ptr<unary_expression> ret = create_node<unary_expression>( token_attr::null() );
	ret->expr = build( v.expr, ctxt );
	ret->op = build_operator( v.preop, operators::none );
	return shared_ptr<expression>(ret);
}

shared_ptr<expression> syntax_tree_builder::build( const sasl::parser_tree::paren_expression& v, ast_builder_context& ctxt )
{
	return build( v.expr, ctxt );
}

shared_ptr<statement> syntax_tree_builder::build( const sasl::parser_tree::statement& v, ast_builder_context& ctxt )
{
	statement_visitor visitor(this, ctxt);
	return boost::apply_visitor(visitor, v);
}

shared_ptr<statement> syntax_tree_builder::build( const sasl::parser_tree::if_statement& v, ast_builder_context& ctxt )
{
	shared_ptr<if_statement> ret = create_node<if_statement>(v.if_keyword.make_copy());
	ret->cond = build(v.cond, ctxt);
	ret->yes_stmt = build(v.stmt, ctxt);
	if (v.else_part){
		ret->no_stmt = build( boost::fusion::at_c<1>(*(v.else_part)), ctxt );
	}
	return ret;
}

shared_ptr<program> syntax_tree_builder::build( const sasl::parser_tree::program& v )
{
	
	shared_ptr<program> prog = create_node<program>( std::string("test") );
	
	ast_builder_context ctxt;
	ctxt.parent = prog;

	BOOST_FOREACH( const sasl::parser_tree::declaration& pt_decl, v ){
		shared_ptr<declaration> gen_decl = build(pt_decl, ctxt);
		if( gen_decl ){
			// if variable declaration with multiple declarator,
			// it will push itself into parent container and return null.
			prog->decls.push_back( gen_decl );
		}
	}
	return prog;
}

shared_ptr<declaration> syntax_tree_builder::build( const sasl::parser_tree::declaration& v, ast_builder_context& ctxt )
{
	dispatch_visitor< shared_ptr<declaration> > visitor(this, ctxt);
	return boost::apply_visitor( visitor, v );
}

shared_ptr<declaration> syntax_tree_builder::build( const sasl::parser_tree::function_definition& /*v*/, ast_builder_context& /*ctxt*/ )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return shared_ptr<declaration>();
}

boost::shared_ptr<declaration> syntax_tree_builder::build( const sasl::parser_tree::basic_declaration& v, ast_builder_context& ctxt )
{
	if (v.decl_body){
		dispatch_visitor< shared_ptr<declaration> > visitor(this, ctxt);
		return boost::apply_visitor( visitor, *v.decl_body );
	}
	return shared_ptr<declaration>();
}

boost::shared_ptr<declaration> syntax_tree_builder::build( const sasl::parser_tree::variable_declaration& /*v*/, ast_builder_context& /*ctxt*/ )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return shared_ptr<declaration>();
}

boost::shared_ptr<declaration> syntax_tree_builder::build( const sasl::parser_tree::function_declaration& /*v*/, ast_builder_context& /*ctxt*/ )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return shared_ptr<declaration>();
}

boost::shared_ptr<declaration> syntax_tree_builder::build( const sasl::parser_tree::typedef_declaration& /*v*/, ast_builder_context& /*ctxt*/ )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return shared_ptr<declaration>();
}

boost::shared_ptr<declaration> syntax_tree_builder::build( const sasl::parser_tree::struct_declaration& /*v*/, ast_builder_context& /*ctxt*/ )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return shared_ptr<declaration>();
}

boost::shared_ptr<type_specifier> syntax_tree_builder::build( const sasl::parser_tree::declaration_specifier& /*v*/, ast_builder_context& /*ctxt*/ )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return shared_ptr<type_specifier>();
}

END_NS_SASL_SYNTAX_TREE()