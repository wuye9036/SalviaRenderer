#ifndef SASL_SYNTAX_TREE_SYNTAX_TREE_BUILDER_H
#define SASL_SYNTAX_TREE_SYNTAX_TREE_BUILDER_H

#include "constant.h"
#include "operator_literal.h"
#include "expression.h"

#include "../parser_tree/literal.h"
#include "../parser_tree/expression.h"
#include <boost/variant/static_visitor.hpp>

class syntax_tree_builder{
public:
	virtual constant::handle_t build_constant( const sasl::parser_tree::constant& v ) = 0;
	virtual operator_literal::handle_t build_operator( const sasl::parser_tree::operator_literal& v ) = 0;
	virtual identifier::handle_t build_identifier( const sasl::parser_tree::identifier_literal& v) = 0;

	virtual expression::handle_t build( const sasl::parser_tree::unary_expression& v) = 0;
	virtual expression::handle_t build( const sasl::parser_tree::cast_expression& v) = 0;
	virtual expression::handle_t build( const sasl::parser_tree::mul_expression& v) = 0;
	virtual expression::handle_t build( const sasl::parser_tree::add_expression& v) = 0;
	virtual expression::handle_t build( const sasl::parser_tree::shf_expression& v) = 0;
	virtual expression::handle_t build( const sasl::parser_tree::rel_expression& v) = 0;
	virtual expression::handle_t build( const sasl::parser_tree::eql_expression& v) = 0;
	virtual expression::handle_t build( const sasl::parser_tree::band_expression& v) = 0;
	virtual expression::handle_t build( const sasl::parser_tree::bxor_expression& v) = 0;
	virtual expression::handle_t build( const sasl::parser_tree::bor_expression& v) = 0;
	virtual expression::handle_t build( const sasl::parser_tree::land_expression& v) = 0;
	virtual expression::handle_t build( const sasl::parser_tree::lor_expression&  v) = 0;
	virtual expression::handle_t build( const sasl::parser_tree::rhs_expression& v) = 0;
	virtual expression::handle_t build( const sasl::parser_tree::assign_expression& v) = 0;
	virtual expression::handle_t build( const sasl::parser_tree::expression& v) = 0;
	virtual expression::handle_t build( const sasl::parser_tree::expression_post& v) = 0;
	virtual expression::handle_t build( const sasl::parser_tree::pm_expression& v ) = 0;
	virtual expression::handle_t build( const sasl::parser_tree::typecast_expression& v) = 0;
	virtual expression::handle_t build( const sasl::parser_tree::post_expression& v) = 0;
	virtual expression::handle_t build( const sasl::parser_tree::cond_expression& v) = 0;
	virtual expression::handle_t build( const sasl::parser_tree::idx_expression& v) = 0;
	virtual expression::handle_t build( const sasl::parser_tree::call_expression& v) = 0;
	virtual expression::handle_t build( const sasl::parser_tree::mem_expression& v) = 0;
	virtual expression::handle_t build( const sasl::parser_tree::unaried_expression& v) = 0;
	virtual expression::handle_t build( const sasl::parser_tree::paren_expression& v ) = 0;
};

struct expression_visitor: public boost::static_visitor<expression::handle_t>{
	expression_visitor( syntax_tree_builder* builder ): builder(builder){}

	expression::handle_t operator()( const int& v){
		assert(false);
		return expression::handle_t();
	}

	expression::handle_t operator()( const sasl::parser_tree::constant& v ){
		constant_expression* ret = new constant_expression();
		ret->value = builder->build_constant(v);
		return expression::handle_t(ret);
	}

	template <typename ExpressionT>
	expression::handle_t operator()( const ExpressionT& v ){
		return builder->build( v );
	}

	syntax_tree_builder* builder;
};

struct post_expression_visitor: public boost::static_visitor< expression::handle_t >{
	post_expression_visitor( syntax_tree_builder* builder, expression::handle_t expr )
		:builder(builder), expr(expr){}

	expression::handle_t operator()( const sasl::parser_tree::idx_expression& v ){
		return composite<index_expression>(v);
	}

	expression::handle_t operator()( const sasl::parser_tree::call_expression& v){
		return composite<call_expression>(v);
	}

	expression::handle_t operator()( const sasl::parser_tree::mem_expression& v){
		return composite<member_expression>(v);
	}

	expression::handle_t operator()( const sasl::parser_tree::operator_literal& v){
		operator_literal::handle_t op = builder->build_operator(v);
		unary_expression* pnode = new unary_expression();
		pnode->op = op;
		pnode->expr = expr;
		return expression::handle_t(pnode);
	}
private:
	template< typename STPostExpressionT, typename ExpressionPostT  >
	expression::handle_t composite( const ExpressionPostT& v ){
		expression::handle_t parent_node = builder->build(v);
		STPostExpressionT* pnode = static_cast<STPostExpressionT*>(parent_node.get());
		pnode->expr = expr;
		return parent_node;
	}

	syntax_tree_builder* builder;
	expression::handle_t expr;
};

struct syntax_tree_builder_impl: public syntax_tree_builder{
	/*******************/
	/* terminals build */
	/*******************/
	operator_literal::handle_t build_operator( const sasl::parser_tree::operator_literal& v ){
		operator_literal* ret = new operator_literal();
		ret->tok.reset( new token_attr(v) );
		return operator_literal::handle_t(ret);
	}

	constant::handle_t build_constant( const sasl::parser_tree::constant& v ){
		constant* ret = new constant();
		ret->tok.reset( new token_attr(v) );
		return constant::handle_t(ret);
	}

	identifier::handle_t build_identifier( const sasl::parser_tree::identifier_literal& v){
		identifier* ret = new identifier();
		ret->tok.reset(new token_attr(v));
		return identifier::handle_t( ret );
	}

	/*************************/
	/* non-terminals builder */
	/*************************/
	expression::handle_t build( const sasl::parser_tree::unary_expression& v){
		return build_variant_expression(v);
	}

	expression::handle_t build( const sasl::parser_tree::cast_expression& v){
		return build_variant_expression(v);
	}
	expression::handle_t build( const sasl::parser_tree::mul_expression& v){
		return build_binary_expression(v);
	}

	expression::handle_t build( const sasl::parser_tree::add_expression& v ){
		return build_binary_expression(v);
	}

	expression::handle_t build( const sasl::parser_tree::shf_expression& v ){
		return build_binary_expression(v);
	}
	expression::handle_t build( const sasl::parser_tree::rel_expression& v){
		return build_binary_expression(v);
	}
	expression::handle_t build( const sasl::parser_tree::eql_expression& v ){
		return build_binary_expression(v);
	}
	expression::handle_t build( const sasl::parser_tree::band_expression& v){
		return build_binary_expression( v );
	}
	expression::handle_t build( const sasl::parser_tree::bxor_expression& v){
		return build_binary_expression( v );
	}
	expression::handle_t build( const sasl::parser_tree::bor_expression& v){
		return build_binary_expression( v );
	}
	expression::handle_t build( const sasl::parser_tree::land_expression& v){
		return build_binary_expression( v );
	}
	expression::handle_t build( const sasl::parser_tree::lor_expression& v){
		return build_binary_expression( v );
	}
	expression::handle_t build( const sasl::parser_tree::rhs_expression& v){
		return build_variant_expression( v );
	}
	expression::handle_t build( const sasl::parser_tree::assign_expression& v){
		return build_right_combine_binary_expression( v );
	}
	expression::handle_t build( const sasl::parser_tree::expression& v ){
		if( v.follow_exprs.empty() ){
			return build( v.first_expr );
		}

		expression_list* ret = new expression_list();
		ret->exprs.push_back( build(v.first_expr) );
		for( sasl::parser_tree::expression_lst::expr_list_t::const_iterator it = v.follow_exprs.begin(); it != v.follow_exprs.end(); ++it ){
			expression::handle_t expr = build(boost::fusion::at_c<1>(*it));
			ret->exprs.push_back( expr );
		}
		return expression_list::handle_t( ret );
	}
	expression::handle_t build( const sasl::parser_tree::expression_post& v){
		return build_variant_expression(v);
	}

	expression::handle_t build( const sasl::parser_tree::pm_expression& v ){
		expression_visitor visitor( this );
		return boost::apply_visitor( visitor, v );
	}

	expression::handle_t build( const sasl::parser_tree::typecast_expression& v){
		cast_expression* ret = new cast_expression();
		ret->type_ident = build_identifier( v.ident );
		ret->expr = build( v.expr );
		return expression::handle_t( ret );
	}
	expression::handle_t build( const sasl::parser_tree::post_expression& v ){
		expression::handle_t expr = build(v.expr);
		for(size_t i = 0; i < v.post.size(); ++i){
			expr = append_expression_post( expr, v.post[i] );
		}
		return expr;
	}
	expression::handle_t build( const sasl::parser_tree::cond_expression& v){
		cond_expression* ret = new cond_expression();
		ret->cond_expr = build(v.condexpr);
		ret->yes_expr = build( boost::fusion::at_c<1>(v.branchexprs) );
		ret->no_expr = build( boost::fusion::at_c<3>(v.branchexprs) );
		return expression::handle_t( ret );
	}
	expression::handle_t build( const sasl::parser_tree::idx_expression& v){
		index_expression* ret = new index_expression();
		ret->idxexpr = build(v.expr);
		return expression::handle_t( ret );
	}
	expression::handle_t build( const sasl::parser_tree::call_expression& v ){
		call_expression* ret = new call_expression();
		if( v.args ){
			expression::handle_t param_exprs = build(*(v.args));
			expression_list* exprlst_ptr = dynamic_cast<expression_list*>( param_exprs.get() );
			if( exprlst_ptr == NULL ){
				ret->params.push_back( param_exprs );
			} else {
				ret->params = exprlst_ptr->exprs;
			}
		}
		return expression::handle_t( ret );
	}
	expression::handle_t build( const sasl::parser_tree::mem_expression& v ){
		member_expression* ret = new member_expression();
		ret->member_ident = build_identifier( v.ident );
		return expression::handle_t(ret);
	}
	expression::handle_t build( const sasl::parser_tree::unaried_expression& v){
		unary_expression* ret = new unary_expression();
		ret->expr = build( v.expr );
		ret->op = build_operator( v.preop );
		return expression::handle_t(ret);
	}

	expression::handle_t build( const sasl::parser_tree::paren_expression& v ){
		return build( v.expr );
	}

private:
	template <typename ExpressionT>
	expression::handle_t build_variant_expression( const ExpressionT& v ){
		expression_visitor visitor(this);
		return boost::apply_visitor( visitor, v );
	}

	template <typename BinaryExpressionT>
	expression::handle_t build_binary_expression( const BinaryExpressionT& v ){
		expression::handle_t ret_expr = build( v.first_expr );
		for (size_t i_exprlist = 0; i_exprlist < v.follow_exprs.size(); ++i_exprlist){
			operator_literal::handle_t op = build_operator(boost::fusion::at_c<0>(v.follow_exprs[i_exprlist]));
			expression::handle_t expr = build( boost::fusion::at_c<1>( v.follow_exprs[i_exprlist] ));
			ret_expr = composite_binary_expression( ret_expr, op, expr );
		}
		return ret_expr;
	}
	template <typename RightCombineBinaryExpressionT>
	expression::handle_t build_right_combine_binary_expression( const RightCombineBinaryExpressionT& v ){
		return build_right_combine_binary_expression( v.first_expr, v.follow_exprs.begin(), v.follow_exprs.end() );
	}
	template <typename ExpressionListIteratorT, typename ExpressionT>
	expression::handle_t build_right_combine_binary_expression( const ExpressionT& lexpr, ExpressionListIteratorT follow_begin, ExpressionListIteratorT follow_end ){
		expression::handle_t left_expr = build( lexpr );
		if( follow_begin == follow_end ){
			return left_expr;
		}
		expression::handle_t right_expr = build_right_combine_binary_expression( boost::fusion::at_c<1>(*follow_begin), follow_begin + 2, follow_end );
		operator_literal::handle_t op = build_operator( boost::fusion::at_c<0>(*follow_begin) );
		return composite_binary_expression( left_expr, op, right_expr );
	}

	expression::handle_t composite_binary_expression( const expression::handle_t& lexpr, const operator_literal::handle_t& op, const expression::handle_t& rexpr){
		binary_expression* composited_expr = new binary_expression();
		composited_expr->left_expr = lexpr;
		composited_expr->right_expr = rexpr;
		composited_expr->op = op;
		return expression::handle_t(composited_expr);
	}
	template <typename ExpressionPostT>
	expression::handle_t append_expression_post( const expression::handle_t expr, const ExpressionPostT& post ){
		post_expression_visitor visitor( this, expr );
		return boost::apply_visitor( visitor, post );
	}
};

#endif