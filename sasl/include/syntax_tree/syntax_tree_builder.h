#ifndef SASL_SYNTAX_TREE_BUILDER_H
#define SASL_SYNTAX_TREE_BUILDER_H

#include <sasl/include/syntax_tree/syntax_tree_fwd.h>

#include <sasl/include/parser_tree/program.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/expression.h>
#include <sasl/include/syntax_tree/statement.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <boost/variant.hpp>
#include <eflib/include/platform/boost_end.h>

BEGIN_NS_SASL_SYNTAX_TREE();

class syntax_tree_builder;

template<typename ResultT>
struct parser_tree_variant_visitor: public boost::static_visitor< ResultT >{
	typedef parser_tree_variant_visitor<ResultT> base_visitor;
	parser_tree_variant_visitor( syntax_tree_builder* builder ): builder(builder){}
protected:
	syntax_tree_builder* builder;
};

// normal expression variant extractor
struct expression_variant_visitor: public parser_tree_variant_visitor< boost::shared_ptr<expression> >{
	typedef token_attr constant_t;

	expression_variant_visitor( syntax_tree_builder* builder );

	// this is an error condition.
	boost::shared_ptr<expression> operator()( const int& v);

	// for constant
	boost::shared_ptr<expression> operator()( const constant_t& v );

	// for normal expression
	template <typename ExpressionT>	boost::shared_ptr<expression> operator()( const ExpressionT& v );

private:
	syntax_tree_builder* builder;
};

struct postfix_qualified_expression_visitor: public parser_tree_variant_visitor< boost::shared_ptr<expression> >{
	postfix_qualified_expression_visitor( syntax_tree_builder* builder, boost::shared_ptr<expression> expr );

	boost::shared_ptr<expression> operator()( const sasl::parser_tree::idx_expression& v );
	boost::shared_ptr<expression> operator()( const sasl::parser_tree::call_expression& v);
	boost::shared_ptr<expression> operator()( const sasl::parser_tree::mem_expression& v);
	boost::shared_ptr<expression> operator()( const token_attr& v);

private:
	template< typename STPostExpressionT, typename ExpressionPostT  >
	boost::shared_ptr<expression> composite( const ExpressionPostT& v );

	boost::shared_ptr<expression> expr;
};

struct statement_variant_visitor: public parser_tree_variant_visitor< boost::shared_ptr<statement> >{
	statement_variant_visitor( syntax_tree_builder* builder );
	template <typename StatementT>
	boost::shared_ptr<statement> operator() ( const StatementT& v ){
		return shared_ptr<statement>(); //builder->build(v);
	}
};
struct syntax_tree_builder{
	/*******************/
	/* terminals build */
	/*******************/
	operators build_operator( const sasl::common::token_attr& v, operators modifier = operators::none );
	boost::shared_ptr<constant_expression> build_constant( const sasl::common::token_attr& v );

	/*************************/
	/* non-terminals builder */
	/*************************/

	//build expressions
	boost::shared_ptr<expression> build( const sasl::parser_tree::unary_expression& v);
	boost::shared_ptr<expression> build( const sasl::parser_tree::cast_expression& v);
	boost::shared_ptr<expression> build( const sasl::parser_tree::mul_expression& v);
	boost::shared_ptr<expression> build( const sasl::parser_tree::add_expression& v );
	boost::shared_ptr<expression> build( const sasl::parser_tree::shf_expression& v );
	boost::shared_ptr<expression> build( const sasl::parser_tree::rel_expression& v);
	boost::shared_ptr<expression> build( const sasl::parser_tree::eql_expression& v );
	boost::shared_ptr<expression> build( const sasl::parser_tree::band_expression& v);
	boost::shared_ptr<expression> build( const sasl::parser_tree::bxor_expression& v);
	boost::shared_ptr<expression> build( const sasl::parser_tree::bor_expression& v);
	boost::shared_ptr<expression> build( const sasl::parser_tree::land_expression& v);
	boost::shared_ptr<expression> build( const sasl::parser_tree::lor_expression& v);
	boost::shared_ptr<expression> build( const sasl::parser_tree::rhs_expression& v);
	boost::shared_ptr<expression> build( const sasl::parser_tree::assign_expression& v);
	boost::shared_ptr<expression> build( const sasl::parser_tree::expression& v );
	boost::shared_ptr<expression> build( const sasl::parser_tree::expression_post& v);
	boost::shared_ptr<expression> build( const sasl::parser_tree::pm_expression& v );
	boost::shared_ptr<expression> build( const sasl::parser_tree::typecast_expression& v);
	boost::shared_ptr<expression> build( const sasl::parser_tree::post_expression& v );
	boost::shared_ptr<expression> build( const sasl::parser_tree::cond_expression& v);
	boost::shared_ptr<expression> build( const sasl::parser_tree::idx_expression& v);
	boost::shared_ptr<expression> build( const sasl::parser_tree::call_expression& v );
	boost::shared_ptr<expression> build( const sasl::parser_tree::mem_expression& v );
	boost::shared_ptr<expression> build( const sasl::parser_tree::unaried_expression& v);
	boost::shared_ptr<expression> build( const sasl::parser_tree::paren_expression& v );

	// build statements
	boost::shared_ptr<statement> build( const sasl::parser_tree::statement& v);
	boost::shared_ptr<statement> build( const sasl::parser_tree::if_statement& v );
	
private:
	template <typename ExpressionT>
	boost::shared_ptr<expression> build_variant_expression( const ExpressionT& v ){
		expression_variant_visitor visitor(this);
		return boost::apply_visitor( visitor, v );
	}

	template <typename BinaryExpressionT>
	boost::shared_ptr<expression> build_binary_expression( const BinaryExpressionT& v ){
		boost::shared_ptr<expression> ret_expr = build( v.first_expr );
		for (size_t i_exprlist = 0; i_exprlist < v.follow_exprs.size(); ++i_exprlist){
			operators op = build_operator(boost::fusion::at_c<0>(v.follow_exprs[i_exprlist]));
			boost::shared_ptr<expression> expr = build( boost::fusion::at_c<1>( v.follow_exprs[i_exprlist] ));
			// ret_expr = composite_binary_expression( ret_expr, op, expr );
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
		operators op = build_operator( boost::fusion::at_c<0>(*follow_begin) );
		return boost::shared_ptr<expression>();//composite_binary_expression( left_expr, op, right_expr );
	}

	boost::shared_ptr<expression> composite_binary_expression( const boost::shared_ptr<expression>& lexpr, const boost::shared_ptr<token_attr>& op, const boost::shared_ptr<expression>& rexpr){
		boost::shared_ptr<binary_expression> composited_expr = create_node<binary_expression>(op);
		composited_expr->left_expr = lexpr;
		composited_expr->right_expr = rexpr;
		composited_expr->op = build_operator(*op, operators::none);

		return composited_expr->typed_handle<expression>();
	}
	template <typename ExpressionPostT>
	boost::shared_ptr<expression> append_expression_post( const boost::shared_ptr<expression> expr, const ExpressionPostT& post ){
		postfix_qualified_expression_visitor visitor( this, expr );
		return boost::apply_visitor( visitor, post );
	}

	boost::shared_ptr<statement> build_for_initializer( const sasl::parser_tree::for_initializer& v, for_statement* for_stmt ){
		// for_stmt->looper.for_init
	}
};

END_NS_SASL_SYNTAX_TREE()

#endif