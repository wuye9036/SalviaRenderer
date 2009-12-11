#include "../../include/syntax_tree/expression.h"
#include "../../include/syntax_tree/constant.h"
#include "../../include/syntax_tree/operator_literal.h"

using namespace boost;
using namespace boost::fusion;

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