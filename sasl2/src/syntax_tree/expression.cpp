#include "../../include/syntax_tree/expression.h"
#include "../../include/syntax_tree/constant.h"
#include "../../include/syntax_tree/operator_literal.h"

using namespace boost;
using namespace boost::fusion;

binary_expression::binary_expression()
: non_terminal<binary_expression>( syntax_node_types::node, token_attr_handle() )
{
}

//operator_literal::handle_t binary_expression::op(){
//	return children[1];
//}
//
//constant::handle_t binary_expression::left_expr(){
//	return children[0];
//}
//
//constant::handle_t binary_expression::right_expr(){
//	return children[2];
//}