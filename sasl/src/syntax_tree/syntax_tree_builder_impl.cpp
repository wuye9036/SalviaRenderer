#include <sasl/include/syntax_tree/syntax_tree_builder_impl.h>

BEGIN_NS_SASL_SYNTAX_TREE()

expression_variant_visitor::expression_variant_visitor( syntax_tree_builder* builder )
: base_visitor(builder){
}

boost::shared_ptr<expression> expression_variant_visitor::operator()( const int& v )
{
	assert(false);
	return boost::shared_ptr<expression>();
}

boost::shared_ptr<expression> expression_variant_visitor::operator()( const constant_t& v )
{
	constant_expression* ret = new constant_expression(v);
	ret->value = builder->build_constant(v);
	return boost::shared_ptr<expression>(ret);
}

template <typename ExpressionT>
boost::shared_ptr<expression> expression_variant_visitor::operator()( const ExpressionT& v )
{
	return builder->build( v );
}

postfix_qualified_expression_visitor::postfix_qualified_expression_visitor(
	syntax_tree_builder* builder,
	boost::shared_ptr<expression> expr )
	:base_visitor(builder), expr(expr)
{
}

boost::shared_ptr<expression> postfix_qualified_expression_visitor::operator()( const sasl::parser_tree::idx_expression& v )
{
	return composite<index_expression>(v);
}

boost::shared_ptr<expression> postfix_qualified_expression_visitor::operator()( const sasl::parser_tree::call_expression& v )
{
	return composite<call_expression>(v);
}

boost::shared_ptr<expression> postfix_qualified_expression_visitor::operator()( const sasl::parser_tree::mem_expression& v )
{
	return composite<member_expression>(v);
}

boost::shared_ptr<expression> postfix_qualified_expression_visitor::operator()( const token_attr& v )
{
	boost::shared_ptr<operator_literal> op = builder->build_operator(v);
	unary_expression* pnode = new unary_expression();
	pnode->op = op;
	pnode->expr = expr;
	return boost::shared_ptr<expression>(pnode);
}

template< typename STPostExpressionT, typename ExpressionPostT  >
boost::shared_ptr<expression> postfix_qualified_expression_visitor::composite( const ExpressionPostT& v )
{
	boost::shared_ptr<expression> parent_node = builder->build(v);
	STPostExpressionT* pnode = static_cast<STPostExpressionT*>(parent_node.get());
	pnode->expr = expr;
	return parent_node;
}


operators syntax_tree_builder_impl::build_operator( const sasl::parser_tree::op& v, operators modifier )
{
	if( modifier == operators::unary ){
		return operator_table.instance().find( v, true, false );
	}
	if( modifier == operators::postfix_op){
		return operator_table.instance().find( v, false, true );
	}
	assert( modifier == operators::none );
	return operator_table.instance.find(v);
}

boost::shared_ptr<constant> syntax_tree_builder_impl::build_constant( const token_attr& v )
{
	constant* ret = new constant();
	/* TODO */
	return boost::shared_ptr<constant>(ret);
}

boost::shared_ptr<identifier> syntax_tree_builder_impl::build_identifier( const token_attr& v )
{
	identifier* ret = new identifier();
	ret->name = v.lit;
	return boost::shared_ptr<identifier>( ret );
}

boost::shared_ptr<expression> syntax_tree_builder_impl::build( const sasl::parser_tree::unary_expression& v )
{
	return build_variant_expression(v);
}

boost::shared_ptr<expression> syntax_tree_builder_impl::build( const sasl::parser_tree::cast_expression& v )
{
	return build_variant_expression(v);
}

boost::shared_ptr<expression> syntax_tree_builder_impl::build( const sasl::parser_tree::mul_expression& v )
{
	return build_binary_expression(v);
}

boost::shared_ptr<expression> syntax_tree_builder_impl::build( const sasl::parser_tree::add_expression& v )
{
	return build_binary_expression(v);
}

boost::shared_ptr<expression> syntax_tree_builder_impl::build( const sasl::parser_tree::shf_expression& v )
{
	return build_binary_expression(v);
}

boost::shared_ptr<expression> syntax_tree_builder_impl::build( const sasl::parser_tree::rel_expression& v )
{
	return build_binary_expression(v);
}

boost::shared_ptr<expression> syntax_tree_builder_impl::build( const sasl::parser_tree::eql_expression& v )
{
	return build_binary_expression(v);
}

boost::shared_ptr<expression> syntax_tree_builder_impl::build( const sasl::parser_tree::band_expression& v )
{
	return build_binary_expression( v );
}

boost::shared_ptr<expression> syntax_tree_builder_impl::build( const sasl::parser_tree::bxor_expression& v )
{
	return build_binary_expression( v );
}

boost::shared_ptr<expression> syntax_tree_builder_impl::build( const sasl::parser_tree::bor_expression& v )
{
	return build_binary_expression( v );
}

boost::shared_ptr<expression> syntax_tree_builder_impl::build( const sasl::parser_tree::land_expression& v )
{
	return build_binary_expression( v );
}

boost::shared_ptr<expression> syntax_tree_builder_impl::build( const sasl::parser_tree::lor_expression& v )
{
	return build_binary_expression( v );
}

boost::shared_ptr<expression> syntax_tree_builder_impl::build( const sasl::parser_tree::rhs_expression& v )
{
	return build_variant_expression( v );
}

boost::shared_ptr<expression> syntax_tree_builder_impl::build( const sasl::parser_tree::assign_expression& v )
{
	return build_right_combine_binary_expression( v );
}

boost::shared_ptr<expression> syntax_tree_builder_impl::build( const sasl::parser_tree::expression& v )
{
	if( v.follow_exprs.empty() ){
		return build( v.first_expr );
	}

	expression_list* ret = new expression_list();
	ret->exprs.push_back( build(v.first_expr) );
	for( sasl::parser_tree::expression_lst::expr_list_t::const_iterator it = v.follow_exprs.begin(); it != v.follow_exprs.end(); ++it ){
		boost::shared_ptr<expression> expr = build(boost::fusion::at_c<1>(*it));
		ret->exprs.push_back( expr );
	}
	return boost::shared_ptr<expression_list>( ret );
}

boost::shared_ptr<expression> syntax_tree_builder_impl::build( const sasl::parser_tree::expression_post& v )
{
	return build_variant_expression(v);
}

boost::shared_ptr<expression> syntax_tree_builder_impl::build( const sasl::parser_tree::pm_expression& v )
{
	expression_visitor visitor( this );
	return boost::apply_visitor( visitor, v );
}

boost::shared_ptr<expression> syntax_tree_builder_impl::build( const sasl::parser_tree::typecast_expression& v )
{
	cast_expression* ret = new cast_expression();
	ret->type_ident = build_identifier( v.ident );
	ret->expr = build( v.expr );
	return boost::shared_ptr<expression>( ret );
}

boost::shared_ptr<expression> syntax_tree_builder_impl::build( const sasl::parser_tree::post_expression& v )
{
	boost::shared_ptr<expression> expr = build(v.expr);
	for(size_t i = 0; i < v.post.size(); ++i){
		expr = append_expression_post( expr, v.post[i] );
	}
	return expr;
}

boost::shared_ptr<expression> syntax_tree_builder_impl::build( const sasl::parser_tree::cond_expression& v )
{
	cond_expression* ret = new cond_expression();
	ret->cond_expr = build(v.condexpr);
	ret->yes_expr = build( boost::fusion::at_c<1>(v.branchexprs) );
	ret->no_expr = build( boost::fusion::at_c<3>(v.branchexprs) );
	return boost::shared_ptr<expression>( ret );
}

boost::shared_ptr<expression> syntax_tree_builder_impl::build( const sasl::parser_tree::idx_expression& v )
{
	index_expression* ret = new index_expression();
	ret->idxexpr = build(v.expr);
	return boost::shared_ptr<expression>( ret );
}

boost::shared_ptr<expression> syntax_tree_builder_impl::build( const sasl::parser_tree::call_expression& v )
{
	call_expression* ret = new call_expression();
	if( v.args ){
		boost::shared_ptr<expression> param_exprs = build(*(v.args));
		expression_list* exprlst_ptr = dynamic_cast<expression_list*>( param_exprs.get() );
		if( exprlst_ptr == NULL ){
			ret->params.push_back( param_exprs );
		} else {
			ret->params = exprlst_ptr->exprs;
		}
	}
	return boost::shared_ptr<expression>( ret );
}

boost::shared_ptr<expression> syntax_tree_builder_impl::build( const sasl::parser_tree::mem_expression& v )
{
	member_expression* ret = new member_expression();
	ret->member_ident = build_identifier( v.ident );
	return boost::shared_ptr<expression>(ret);
}

boost::shared_ptr<expression> syntax_tree_builder_impl::build( const sasl::parser_tree::unaried_expression& v )
{
	unary_expression* ret = new unary_expression();
	ret->expr = build( v.expr );
	ret->op = build_operator( v.preop );
	return boost::shared_ptr<expression>(ret);
}

boost::shared_ptr<expression> syntax_tree_builder_impl::build( const sasl::parser_tree::paren_expression& v )
{
	return build( v.expr );
}
END_NS_SASL_SYNTAX_TREE()