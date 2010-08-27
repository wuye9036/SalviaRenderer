#include <sasl/include/syntax_tree/expression.h>
#include <sasl/include/syntax_tree/visitor.h>

using namespace boost;

BEGIN_NS_SASL_SYNTAX_TREE();

expression::expression( syntax_node_types ntype, boost::shared_ptr<token_attr> tok )
	: node( ntype, tok )
{
}

constant_expression::constant_expression( boost::shared_ptr<token_attr> tok )
	: expression( syntax_node_types::constant_expression, tok ), ctype( literal_constant_types::none) { }

void constant_expression::accept( syntax_tree_visitor* v ){
	v->visit( *this );
}

unary_expression::unary_expression( boost::shared_ptr<token_attr> tok )
	: expression( syntax_node_types::unary_expression, tok ), op( operators::none ) { }

void unary_expression::accept( syntax_tree_visitor* v ){
	v->visit( *this );
}

cast_expression::cast_expression( boost::shared_ptr<token_attr> tok )
	: expression( syntax_node_types::cast_expression, tok ){
}

void cast_expression::accept( syntax_tree_visitor* v ){
	v->visit( *this );
}

binary_expression::binary_expression( boost::shared_ptr<token_attr> tok )
	: expression( syntax_node_types::binary_expression, tok ), op( operators::none) { }

void binary_expression::accept( syntax_tree_visitor* v ){
	v->visit( *this );
}

expression_list::expression_list( boost::shared_ptr<token_attr> tok )
	: expression( syntax_node_types::expression_list, tok )
{
}

void expression_list::accept( syntax_tree_visitor* v ){
	v->visit( *this );
}

cond_expression::cond_expression( boost::shared_ptr<token_attr> tok )
	: expression( syntax_node_types::cond_expression, tok ){
}

void cond_expression::accept( syntax_tree_visitor* v ){
	v->visit( *this );
}

index_expression::index_expression( boost::shared_ptr<token_attr> tok )
	: expression( syntax_node_types::index_expression, tok ){
}

void index_expression::accept( syntax_tree_visitor* v ){
	v->visit( *this );
}

call_expression::call_expression( boost::shared_ptr<token_attr> tok )
	: expression( syntax_node_types::call_expression, tok ){
}

void call_expression::accept( syntax_tree_visitor* v ){
	v->visit( *this );
}

member_expression::member_expression( boost::shared_ptr<token_attr> tok )
	: expression( syntax_node_types::member_expression, tok )
{
}

void member_expression::accept( syntax_tree_visitor* v ){
	v->visit( *this );
}

variable_expression::variable_expression( boost::shared_ptr<token_attr> tok )
: expression( syntax_node_types::variable_expression, tok )
{	
}

void variable_expression::accept( syntax_tree_visitor* v )
{
	v->visit( *this );
}

END_NS_SASL_SYNTAX_TREE();