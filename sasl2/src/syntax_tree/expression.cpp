#include "../../include/syntax_tree/expression.h"
#include "../../include/syntax_tree/constant.h"
#include "../../include/syntax_tree/operator_literal.h"

using namespace boost;
using namespace boost::fusion;

binary_expression& binary_expression::operator = ( const boost::fusion::vector< constant, operator_literal, constant > & rhs ){
	left_expr.reset( at_c<0>(rhs).clone<constant>() );
	op = at_c<1>(rhs).op;
	right_expr.reset( at_c<2>(rhs).clone<constant>() );
	return *this;
}

//Ç³¿½±´
binary_expression& binary_expression::operator = ( const binary_expression& rhs ){
	op = rhs.op;
	left_expr = rhs.left_expr;
	right_expr = rhs.right_expr;
	return *this;
}

binary_expression::binary_expression()
: op( ::operators::none ), node( syntax_node_types::node, token_attr() )
{
}

binary_expression::binary_expression( const binary_expression& rhs )
: node( rhs ),
op( rhs.op ),
left_expr( rhs.left_expr ),
right_expr( rhs.right_expr )
{
}

node* binary_expression::clone_impl() const {
	return new binary_expression( *this );
}

node* binary_expression::deepcopy_impl() const{
	binary_expression* ret = new binary_expression();
	ret->op = op;
	ret->left_expr.reset( left_expr->deepcopy<constant>() );
	ret->right_expr.reset( right_expr->deepcopy<constant>() );
	return ret;
}