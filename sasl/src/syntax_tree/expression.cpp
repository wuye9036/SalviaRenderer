#include "../../include/syntax_tree/expression.h"
#include "../../include/syntax_tree/constant.h"
#include "../../include/syntax_tree/operator_literal.h"

using namespace boost;
using namespace boost::fusion;

binary_expression::binary_expression()
: expression( syntax_node_types::node )
{
}