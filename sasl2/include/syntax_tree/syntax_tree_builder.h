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
	virtual expression::handle_t build_binary_expression( const sasl::parser_tree::binary_expression& v ) = 0;
	virtual expression::handle_t build_paren_expression( const sasl::parser_tree::paren_expression& v ) = 0;
	virtual expression::handle_t build_primary_expression( const sasl::parser_tree::primary_expression& v ) = 0;
};

struct expression_visitor: public boost::static_visitor<expression::handle_t>{
	expression_visitor( syntax_tree_builder* builder ): builder(builder){}

	expression::handle_t operator()( const sasl::parser_tree::constant& v ){
		constant_expression* ret = new constant_expression();
		ret->value = builder->build_constant(v);
		return expression::handle_t(ret);
	}

	expression::handle_t operator()( const sasl::parser_tree::paren_expression& v ){
		return builder->build_paren_expression( v );
	}

	syntax_tree_builder* builder;
};

struct syntax_tree_builder_impl: public syntax_tree_builder{
	expression::handle_t build_binary_expression( const sasl::parser_tree::binary_expression& v ){
		expression::handle_t ret_expr = build_primary_expression( v.first_expr );
		for (size_t i_exprlist = 0; i_exprlist < v.follow_exprs.size(); ++i_exprlist){
			operator_literal::handle_t op = build_operator(boost::fusion::at_c<0>(v.follow_exprs[i_exprlist]));
			expression::handle_t expr = build_primary_expression( boost::fusion::at_c<1>( v.follow_exprs[i_exprlist] ));
			ret_expr = composite_binary_expression( ret_expr, op, expr );
		}
		return ret_expr;
	}

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

	expression::handle_t build_primary_expression( const sasl::parser_tree::primary_expression& v ){
		expression_visitor visitor( this );
		return boost::apply_visitor( visitor, v );
	}

	expression::handle_t build_paren_expression( const sasl::parser_tree::paren_expression& v ){
		return build_binary_expression( v.expr );
	}

private:
	expression::handle_t composite_binary_expression( const expression::handle_t& lexpr, const operator_literal::handle_t& op, const expression::handle_t& rexpr){
		binary_expression* composited_expr = new binary_expression();
		composited_expr->left_expr = lexpr;
		composited_expr->right_expr = rexpr;
		composited_expr->op = op;
		return expression::handle_t(composited_expr);
	}
};

#endif