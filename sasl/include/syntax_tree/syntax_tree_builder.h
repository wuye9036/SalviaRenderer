#ifndef SASL_SYNTAX_TREE_SYNTAX_TREE_BUILDER_H
#define SASL_SYNTAX_TREE_SYNTAX_TREE_BUILDER_H

#include "syntax_tree_fwd.h"
#include "constant.h"
#include "expression.h"
#include <sasl/include/parser_tree/literal.h>
#include <sasl/include/parser_tree/expression.h>
#include <boost/variant/static_visitor.hpp>

BEGIN_NS_SASL_SYNTAX_TREE()

using sasl::common::token_attr;

class syntax_tree_builder{
public:
	virtual boost::shared_ptr<sasl::common::token_attr> build_constant( const sasl::common::token_attr& v ) = 0;
	virtual boost::shared_ptr<sasl::common::token_attr> build_operator( const sasl::common::token_attr& v ) = 0;
	virtual boost::shared_ptr<sasl::common::token_attr> build_identifier( const sasl::common::token_attr& v) = 0;

	virtual boost::shared_ptr<expression> build( const sasl::parser_tree::unary_expression& v) = 0;
	virtual boost::shared_ptr<expression> build( const sasl::parser_tree::cast_expression& v) = 0;
	virtual boost::shared_ptr<expression> build( const sasl::parser_tree::mul_expression& v) = 0;
	virtual boost::shared_ptr<expression> build( const sasl::parser_tree::add_expression& v) = 0;
	virtual boost::shared_ptr<expression> build( const sasl::parser_tree::shf_expression& v) = 0;
	virtual boost::shared_ptr<expression> build( const sasl::parser_tree::rel_expression& v) = 0;
	virtual boost::shared_ptr<expression> build( const sasl::parser_tree::eql_expression& v) = 0;
	virtual boost::shared_ptr<expression> build( const sasl::parser_tree::band_expression& v) = 0;
	virtual boost::shared_ptr<expression> build( const sasl::parser_tree::bxor_expression& v) = 0;
	virtual boost::shared_ptr<expression> build( const sasl::parser_tree::bor_expression& v) = 0;
	virtual boost::shared_ptr<expression> build( const sasl::parser_tree::land_expression& v) = 0;
	virtual boost::shared_ptr<expression> build( const sasl::parser_tree::lor_expression&  v) = 0;
	virtual boost::shared_ptr<expression> build( const sasl::parser_tree::rhs_expression& v) = 0;
	virtual boost::shared_ptr<expression> build( const sasl::parser_tree::assign_expression& v) = 0;
	virtual boost::shared_ptr<expression> build( const sasl::parser_tree::expression& v) = 0;
	virtual boost::shared_ptr<expression> build( const sasl::parser_tree::expression_post& v) = 0;
	virtual boost::shared_ptr<expression> build( const sasl::parser_tree::pm_expression& v ) = 0;
	virtual boost::shared_ptr<expression> build( const sasl::parser_tree::typecast_expression& v) = 0;
	virtual boost::shared_ptr<expression> build( const sasl::parser_tree::post_expression& v) = 0;
	virtual boost::shared_ptr<expression> build( const sasl::parser_tree::cond_expression& v) = 0;
	virtual boost::shared_ptr<expression> build( const sasl::parser_tree::idx_expression& v) = 0;
	virtual boost::shared_ptr<expression> build( const sasl::parser_tree::call_expression& v) = 0;
	virtual boost::shared_ptr<expression> build( const sasl::parser_tree::mem_expression& v) = 0;
	virtual boost::shared_ptr<expression> build( const sasl::parser_tree::unaried_expression& v) = 0;
	virtual boost::shared_ptr<expression> build( const sasl::parser_tree::paren_expression& v ) = 0;
};

struct expression_visitor: public boost::static_visitor<boost::shared_ptr<expression>>{
	expression_visitor( syntax_tree_builder* builder ): builder(builder){}

	boost::shared_ptr<expression> operator()( const int& v){
		assert(false);
		return boost::shared_ptr<expression>();
	}

	boost::shared_ptr<expression> operator()( const sasl::parser_tree::constant& v ){
		constant_expression* ret = new constant_expression();
		ret->value = builder->build_constant(v);
		return boost::shared_ptr<expression>(ret);
	}

	template <typename ExpressionT>
	boost::shared_ptr<expression> operator()( const ExpressionT& v ){
		return builder->build( v );
	}

	syntax_tree_builder* builder;
};

struct post_expression_visitor: public boost::static_visitor< boost::shared_ptr<expression> >{
	post_expression_visitor( syntax_tree_builder* builder, boost::shared_ptr<expression> expr )
		:builder(builder), expr(expr){}

	boost::shared_ptr<expression> operator()( const sasl::parser_tree::idx_expression& v ){
		return composite<index_expression>(v);
	}

	boost::shared_ptr<expression> operator()( const sasl::parser_tree::call_expression& v){
		return composite<call_expression>(v);
	}

	boost::shared_ptr<expression> operator()( const sasl::parser_tree::mem_expression& v){
		return composite<member_expression>(v);
	}

	boost::shared_ptr<expression> operator()( const sasl::parser_tree::operator_literal& v){
		boost::shared_ptr<operator_literal> op = builder->build_operator(v);
		unary_expression* pnode = new unary_expression();
		pnode->op = op;
		pnode->expr = expr;
		return boost::shared_ptr<expression>(pnode);
	}
private:
	template< typename STPostExpressionT, typename ExpressionPostT  >
	boost::shared_ptr<expression> composite( const ExpressionPostT& v ){
		boost::shared_ptr<expression> parent_node = builder->build(v);
		STPostExpressionT* pnode = static_cast<STPostExpressionT*>(parent_node.get());
		pnode->expr = expr;
		return parent_node;
	}

	syntax_tree_builder* builder;
	boost::shared_ptr<expression> expr;
};

struct syntax_tree_builder_impl: public syntax_tree_builder{
	/*******************/
	/* terminals build */
	/*******************/
	boost::shared_ptr<operator_literal> build_operator( const sasl::parser_tree::operator_literal& v ){
		operator_literal* ret = new operator_literal();
		ret->tok.reset( new token_attr(v) );
		return boost::shared_ptr<operator_literal>(ret);
	}

	boost::shared_ptr<constant> build_constant( const sasl::parser_tree::constant& v ){
		constant* ret = new constant();
		ret->tok.reset( new token_attr(v) );
		return boost::shared_ptr<constant>(ret);
	}

	boost::shared_ptr<identifier> build_identifier( const sasl::parser_tree::identifier_literal& v){
		identifier* ret = new identifier();
		ret->tok.reset(new token_attr(v));
		return boost::shared_ptr<identifier>( ret );
	}

	/*************************/
	/* non-terminals builder */
	/*************************/
	boost::shared_ptr<expression> build( const sasl::parser_tree::unary_expression& v){
		return build_variant_expression(v);
	}

	boost::shared_ptr<expression> build( const sasl::parser_tree::cast_expression& v){
		return build_variant_expression(v);
	}
	boost::shared_ptr<expression> build( const sasl::parser_tree::mul_expression& v){
		return build_binary_expression(v);
	}

	boost::shared_ptr<expression> build( const sasl::parser_tree::add_expression& v ){
		return build_binary_expression(v);
	}

	boost::shared_ptr<expression> build( const sasl::parser_tree::shf_expression& v ){
		return build_binary_expression(v);
	}
	boost::shared_ptr<expression> build( const sasl::parser_tree::rel_expression& v){
		return build_binary_expression(v);
	}
	boost::shared_ptr<expression> build( const sasl::parser_tree::eql_expression& v ){
		return build_binary_expression(v);
	}
	boost::shared_ptr<expression> build( const sasl::parser_tree::band_expression& v){
		return build_binary_expression( v );
	}
	boost::shared_ptr<expression> build( const sasl::parser_tree::bxor_expression& v){
		return build_binary_expression( v );
	}
	boost::shared_ptr<expression> build( const sasl::parser_tree::bor_expression& v){
		return build_binary_expression( v );
	}
	boost::shared_ptr<expression> build( const sasl::parser_tree::land_expression& v){
		return build_binary_expression( v );
	}
	boost::shared_ptr<expression> build( const sasl::parser_tree::lor_expression& v){
		return build_binary_expression( v );
	}
	boost::shared_ptr<expression> build( const sasl::parser_tree::rhs_expression& v){
		return build_variant_expression( v );
	}
	boost::shared_ptr<expression> build( const sasl::parser_tree::assign_expression& v){
		return build_right_combine_binary_expression( v );
	}
	boost::shared_ptr<expression> build( const sasl::parser_tree::expression& v ){
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
	boost::shared_ptr<expression> build( const sasl::parser_tree::expression_post& v){
		return build_variant_expression(v);
	}

	boost::shared_ptr<expression> build( const sasl::parser_tree::pm_expression& v ){
		expression_visitor visitor( this );
		return boost::apply_visitor( visitor, v );
	}

	boost::shared_ptr<expression> build( const sasl::parser_tree::typecast_expression& v){
		cast_expression* ret = new cast_expression();
		ret->type_ident = build_identifier( v.ident );
		ret->expr = build( v.expr );
		return boost::shared_ptr<expression>( ret );
	}
	boost::shared_ptr<expression> build( const sasl::parser_tree::post_expression& v ){
		boost::shared_ptr<expression> expr = build(v.expr);
		for(size_t i = 0; i < v.post.size(); ++i){
			expr = append_expression_post( expr, v.post[i] );
		}
		return expr;
	}
	boost::shared_ptr<expression> build( const sasl::parser_tree::cond_expression& v){
		cond_expression* ret = new cond_expression();
		ret->cond_expr = build(v.condexpr);
		ret->yes_expr = build( boost::fusion::at_c<1>(v.branchexprs) );
		ret->no_expr = build( boost::fusion::at_c<3>(v.branchexprs) );
		return boost::shared_ptr<expression>( ret );
	}
	boost::shared_ptr<expression> build( const sasl::parser_tree::idx_expression& v){
		index_expression* ret = new index_expression();
		ret->idxexpr = build(v.expr);
		return boost::shared_ptr<expression>( ret );
	}
	boost::shared_ptr<expression> build( const sasl::parser_tree::call_expression& v ){
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
	boost::shared_ptr<expression> build( const sasl::parser_tree::mem_expression& v ){
		member_expression* ret = new member_expression();
		ret->member_ident = build_identifier( v.ident );
		return boost::shared_ptr<expression>(ret);
	}
	boost::shared_ptr<expression> build( const sasl::parser_tree::unaried_expression& v){
		unary_expression* ret = new unary_expression();
		ret->expr = build( v.expr );
		ret->op = build_operator( v.preop );
		return boost::shared_ptr<expression>(ret);
	}

	boost::shared_ptr<expression> build( const sasl::parser_tree::paren_expression& v ){
		return build( v.expr );
	}

private:
	template <typename ExpressionT>
	boost::shared_ptr<expression> build_variant_expression( const ExpressionT& v ){
		expression_visitor visitor(this);
		return boost::apply_visitor( visitor, v );
	}

	template <typename BinaryExpressionT>
	boost::shared_ptr<expression> build_binary_expression( const BinaryExpressionT& v ){
		boost::shared_ptr<expression> ret_expr = build( v.first_expr );
		for (size_t i_exprlist = 0; i_exprlist < v.follow_exprs.size(); ++i_exprlist){
			boost::shared_ptr<operator_literal> op = build_operator(boost::fusion::at_c<0>(v.follow_exprs[i_exprlist]));
			boost::shared_ptr<expression> expr = build( boost::fusion::at_c<1>( v.follow_exprs[i_exprlist] ));
			ret_expr = composite_binary_expression( ret_expr, op, expr );
		}
		return ret_expr;
	}
	template <typename RightCombineBinaryExpressionT>
	boost::shared_ptr<expression> build_right_combine_binary_expression( const RightCombineBinaryExpressionT& v ){
		return build_right_combine_binary_expression( v.first_expr, v.follow_exprs.begin(), v.follow_exprs.end() );
	}
	template <typename ExpressionListIteratorT, typename ExpressionT>
	boost::shared_ptr<expression> build_right_combine_binary_expression( const ExpressionT& lexpr, ExpressionListIteratorT follow_begin, ExpressionListIteratorT follow_end ){
		boost::shared_ptr<expression> left_expr = build( lexpr );
		if( follow_begin == follow_end ){
			return left_expr;
		}
		boost::shared_ptr<expression> right_expr = build_right_combine_binary_expression( boost::fusion::at_c<1>(*follow_begin), follow_begin + 2, follow_end );
		boost::shared_ptr<operator_literal> op = build_operator( boost::fusion::at_c<0>(*follow_begin) );
		return composite_binary_expression( left_expr, op, right_expr );
	}

	boost::shared_ptr<expression> composite_binary_expression( const boost::shared_ptr<expression>& lexpr, const boost::shared_ptr<operator_literal>& op, const boost::shared_ptr<expression>& rexpr){
		binary_expression* composited_expr = new binary_expression();
		composited_expr->left_expr = lexpr;
		composited_expr->right_expr = rexpr;
		composited_expr->op = op;
		return boost::shared_ptr<expression>(composited_expr);
	}
	template <typename ExpressionPostT>
	boost::shared_ptr<expression> append_expression_post( const boost::shared_ptr<expression> expr, const ExpressionPostT& post ){
		post_expression_visitor visitor( this, expr );
		return boost::apply_visitor( visitor, post );
	}
};

END_NS_SASL_SYNTAX_TREE()

#endif