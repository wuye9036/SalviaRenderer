#ifndef SASL_PARSER_TREE_EXPRESSION_H
#define SASL_PARSER_TREE_EXPRESSION_H

#include "parser_tree_forward.h"
#include <sasl/include/common/token_attr.h>
#include <boost/variant.hpp>
#include <boost/fusion/adapted.hpp>
#include <boost/fusion/tuple.hpp>
#include <boost/optional.hpp>
#include <vector>

BEGIN_NS_SASL_PARSER_TREE()

struct paren_expression;
struct idx_expression;
struct call_expression;
struct mem_expression;
struct unaried_expression;
struct typecast_expression;
struct cond_expression;
struct post_expression;

template <typename BinaryExpressionT> struct binary_expression;

typedef boost::variant<
	int,
	boost::recursive_wrapper<post_expression>,
	boost::recursive_wrapper<unaried_expression> >	unary_expression;
typedef boost::variant<
	unary_expression,
	boost::recursive_wrapper<typecast_expression> >	cast_expression;

typedef binary_expression<cast_expression>		mul_expression;
typedef binary_expression<mul_expression>		add_expression;
typedef binary_expression<add_expression>		shf_expression;
typedef binary_expression<shf_expression>		rel_expression;
typedef binary_expression<rel_expression>		eql_expression;
typedef binary_expression<eql_expression>		band_expression;
typedef binary_expression<band_expression>		bxor_expression;
typedef binary_expression<bxor_expression>		bor_expression;
typedef binary_expression<bor_expression>		land_expression;
typedef binary_expression<land_expression>		lor_expression;

typedef boost::variant<
	int,
	boost::recursive_wrapper<cond_expression>,
	boost::recursive_wrapper<lor_expression> >	rhs_expression;

typedef binary_expression<rhs_expression>		assign_expression;
typedef binary_expression<assign_expression>	expression_lst;
typedef expression_lst expression;

template <typename ChildExpressionT>
struct binary_expression {
	typedef ChildExpressionT child_expression_t;
	typedef std::vector<boost::fusion::vector< token_attr, child_expression_t> > expr_list_t;
	child_expression_t first_expr;
	expr_list_t follow_exprs;
};

typedef boost::variant<
	boost::recursive_wrapper<idx_expression>,
	boost::recursive_wrapper<call_expression>,
	boost::recursive_wrapper<mem_expression>,
	token_attr >								expression_post;
typedef boost::variant<
	token_attr,
	boost::recursive_wrapper<paren_expression> >	pm_expression;

struct typecast_expression {
	token_attr	lparen;
	token_attr	ident;
	token_attr	rparen;
	expression	expr;
};

struct post_expression{
	pm_expression					expr;
	std::vector<expression_post>	post;
};

struct cond_expression{
	typedef boost::fusion::vector<operator_literal, expression, operator_literal, assign_expression> branches_t;
	lor_expression	condexpr;
	branches_t		branchexprs;
};

struct idx_expression{
	token_attr	lsbracket;
	expression	expr;
	token_attr	rsbracket;
};

struct call_expression{
	token_attr			lparen;
	boost::optional<expression>	args;
	token_attr			rparen;
};

struct mem_expression{
	token_attr	dot;
	token_attr	ident;
};

struct unaried_expression{
	token_attr	preop;
	cast_expression		expr;
};

struct paren_expression {
	token_attr lparen;
	expression expr;
	token_attr rparen;
};

END_NS_SASL_PARSER_TREE()

/////////////////////////////////////////////////////////
//  BOOST_FUSION_ADAPT_STRUCT 需要写在全局作用域中  //
/////////////////////////////////////////////////////////
#define SASL_ADAPT_BINARY_EXPRESSION( CLASS_NAME )	\
	SASL_ADAPT_BINARY_EXPRESSION_IMPL( BOOST_PP_CAT(sasl::parser_tree::,CLASS_NAME) )

#define SASL_ADAPT_BINARY_EXPRESSION_IMPL( NAMESPACED_CLASS_NAME )				\
	BOOST_FUSION_ADAPT_STRUCT(													\
		NAMESPACED_CLASS_NAME,													\
		( BOOST_PP_CAT(NAMESPACED_CLASS_NAME,::child_expression_t), first_expr)	\
		( BOOST_PP_CAT(NAMESPACED_CLASS_NAME,::expr_list_t), follow_exprs)		\
		);

SASL_ADAPT_BINARY_EXPRESSION( add_expression );
SASL_ADAPT_BINARY_EXPRESSION( mul_expression );
SASL_ADAPT_BINARY_EXPRESSION( shf_expression );
SASL_ADAPT_BINARY_EXPRESSION( rel_expression );
SASL_ADAPT_BINARY_EXPRESSION( eql_expression );
SASL_ADAPT_BINARY_EXPRESSION( band_expression );
SASL_ADAPT_BINARY_EXPRESSION( bxor_expression );
SASL_ADAPT_BINARY_EXPRESSION( bor_expression );
SASL_ADAPT_BINARY_EXPRESSION( land_expression );
SASL_ADAPT_BINARY_EXPRESSION( lor_expression );
SASL_ADAPT_BINARY_EXPRESSION( assign_expression );
SASL_ADAPT_BINARY_EXPRESSION( expression_lst );

BOOST_FUSION_ADAPT_STRUCT( sasl::parser_tree::typecast_expression,
						  (sasl::common::token_attr, lparen)
						  (sasl::common::token_attr, ident)
						  (sasl::common::token_attr, rparen)
						  (sasl::parser_tree::expression, expr)
						  );
BOOST_FUSION_ADAPT_STRUCT( sasl::parser_tree::post_expression,
						  (sasl::parser_tree::pm_expression, expr)
						  (std::vector<sasl::parser_tree::expression_post>, post)
						  );
BOOST_FUSION_ADAPT_STRUCT( sasl::parser_tree::cond_expression,
						  (sasl::parser_tree::lor_expression, condexpr)
						  (sasl::parser_tree::cond_expression::branches_t, branchexprs)
						  );
BOOST_FUSION_ADAPT_STRUCT( sasl::parser_tree::idx_expression,
						  (sasl::common::token_attr, lsbracket)
						  (sasl::parser_tree::expression, expr)
						  (sasl::common::token_attr, rsbracket)
						  );
BOOST_FUSION_ADAPT_STRUCT( sasl::parser_tree::call_expression,
						  (sasl::common::token_attr, lparen)
						  (boost::optional<sasl::parser_tree::expression>, args)
						  (sasl::common::token_attr, rparen)
						  );
BOOST_FUSION_ADAPT_STRUCT( sasl::parser_tree::mem_expression,
						  (sasl::common::token_attr, dot)
						  (sasl::common::token_attr, ident)
						  );
BOOST_FUSION_ADAPT_STRUCT( sasl::parser_tree::unaried_expression,
						  (sasl::common::token_attr, preop)
						  (sasl::parser_tree::cast_expression, expr)
						  );
BOOST_FUSION_ADAPT_STRUCT(sasl::parser_tree::paren_expression, 
						  (sasl::common::token_attr, lparen)
						  (sasl::parser_tree::expression, expr)
						  (sasl::common::token_attr, rparen)
						  );
#endif