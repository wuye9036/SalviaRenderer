#ifndef SASL_PARSER_TREE_EXPRESSION_H
#define SASL_PARSER_TREE_EXPRESSION_H

#include "parser_tree_forward.h"
#include "literal.h"
#include <boost/fusion/adapted.hpp>
#include <boost/optional.hpp>

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
typedef expression_lst							expression;



typedef boost::variant<
	boost::recursive_wrapper<idx_expression>,
	boost::recursive_wrapper<call_expression>,
	boost::recursive_wrapper<mem_expression>,
	operator_literal >								expression_post;
typedef boost::variant<
	constant,
	boost::recursive_wrapper<paren_expression> >	pm_expression;

template <typename ChildExpressionT>
struct binary_expression {
	typedef ChildExpressionT child_expression_t;
	typedef std::vector<boost::fusion::vector< operator_literal, child_expression_t> > expr_list_t;
	child_expression_t first_expr;
	expr_list_t follow_exprs;
};

struct typecast_expression {
	operator_literal	lparen;
	identifier_literal	ident;
	operator_literal	rparen;
	expression			expr;
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
	operator_literal	lsbracket;
	expression			expr;
	operator_literal	rsbracket;
};

struct call_expression{
	operator_literal			lparen;
	boost::optional<expression>	args;
	operator_literal			rparen;
};

struct mem_expression{
	operator_literal	dot;
	identifier_literal	ident;
};

struct unaried_expression{
	operator_literal	preop;
	cast_expression		expr;
};

struct paren_expression {
	operator_literal lparen;
	expression expr;
	operator_literal rparen;
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
						  (sasl::parser_tree::operator_literal, lparen)
						  (sasl::parser_tree::identifier_literal, ident)
						  (sasl::parser_tree::operator_literal, rparen)
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
						  (sasl::parser_tree::operator_literal, lsbracket)
						  (sasl::parser_tree::expression, expr)
						  (sasl::parser_tree::operator_literal, rsbracket)
						  );
BOOST_FUSION_ADAPT_STRUCT( sasl::parser_tree::call_expression,
						  (sasl::parser_tree::operator_literal, lparen)
						  (boost::optional<sasl::parser_tree::expression>, args)
						  (sasl::parser_tree::operator_literal, rparen)
						  );
BOOST_FUSION_ADAPT_STRUCT( sasl::parser_tree::mem_expression,
						  (sasl::parser_tree::operator_literal, dot)
						  (sasl::parser_tree::identifier_literal, ident)
						  );
BOOST_FUSION_ADAPT_STRUCT( sasl::parser_tree::unaried_expression,
						  (sasl::parser_tree::operator_literal, preop)
						  (sasl::parser_tree::cast_expression, expr)
						  );
BOOST_FUSION_ADAPT_STRUCT(sasl::parser_tree::paren_expression, 
						  (sasl::parser_tree::operator_literal, lparen)
						  (sasl::parser_tree::expression, expr)
						  (sasl::parser_tree::operator_literal, rparen)
						  );
#endif