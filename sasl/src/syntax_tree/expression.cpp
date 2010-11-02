#include <sasl/include/syntax_tree/expression.h>
#include <sasl/include/syntax_tree/visitor.h>
#include <eflib/include/platform/disable_warnings.h>
#include <boost/assign/std/vector.hpp>
#include <eflib/include/platform/enable_warnings.h>

using namespace boost;
using namespace boost::assign;
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


operators_helper::operators_helper()
{
	prefix_ops +=
		operators::positive, operators::negative,
		operators::bit_not, operators::logic_not,
		operators::prefix_incr, operators::prefix_decr
		;
	postfix_ops +=
		operators::postfix_incr, operators::postfix_decr
		;
	binary_ops +=
		operators::add, operators::sub,
		operators::mul, operators::div, operators::mod,
		operators::assign,
		operators::add_assign, operators::sub_assign,
		operators::mul_assign, operators::div_assign, operators::mod_assign,
		operators::bit_and, operators::bit_or, operators::bit_xor,
		operators::bit_and_assign, operators::bit_or_assign, operators::bit_xor_assign,
		operators::logic_and, operators::logic_or,
		operators::equal, operators::not_equal,
		operators::greater, operators::greater_equal,
		operators::less, operators::less_equal,
		operators::left_shift, operators::right_shift,
		operators::lshift_assign, operators::rshift_assign
		;

}

bool operators_helper::is_prefix( operators op )
{
	return include( prefix_ops, op );
}

bool operators_helper::is_binary( operators op )
{
	return include( binary_ops, op );
}

bool operators_helper::is_postfix( operators op )
{
	return include( postfix_ops, op );
}

bool operators_helper::is_unary( operators op )
{
	return is_prefix(op) || is_postfix(op);
}

operators_helper& operators_helper::instance()
{
	static operators_helper op_helper;
	return op_helper;
}

bool operators_helper::include( const std::vector<operators>& c, operators op)
{
	return std::find( c.begin(), c.end(), op) != c.end();
}

END_NS_SASL_SYNTAX_TREE();