#include <sasl/include/syntax_tree/statement.h>
#include <sasl/include/syntax_tree/visitor.h>
#include <sasl/include/common/token_attr.h>

BEGIN_NS_SASL_SYNTAX_TREE();

statement::statement( syntax_node_types nodetype, boost::shared_ptr<token_attr> tok )
: node( nodetype, tok ){}

boost::shared_ptr<struct label> statement::pop_label()
{
	assert( !labels.empty() );
	boost::shared_ptr<struct label> ret = labels.back();
	labels.pop_back();
	return ret;
}
declaration_statement::declaration_statement( boost::shared_ptr<token_attr> tok )
	: statement( syntax_node_types::declaration_statement, tok ){ }

void declaration_statement::accept( syntax_tree_visitor* v ){
	v->visit( *this );
}

expression_statement::expression_statement( boost::shared_ptr<token_attr> tok )
: statement( syntax_node_types::expression_statement, tok ){
}

void expression_statement::accept( syntax_tree_visitor* v )
{
	v->visit(*this);
}

void compound_statement::accept( syntax_tree_visitor* v ){
	v->visit(*this);
}

compound_statement::compound_statement( boost::shared_ptr<token_attr> tok )
: statement( syntax_node_types::compound_statement, tok ){
}


if_statement::if_statement( boost::shared_ptr<token_attr> tok )
: statement( syntax_node_types::if_statement, tok ){
}

void if_statement::accept( syntax_tree_visitor* v )
{
	v->visit(*this);
}

while_statement::while_statement( boost::shared_ptr<token_attr> tok )
: statement( syntax_node_types::while_statement, tok ) {
}

void while_statement::accept( syntax_tree_visitor* v ){
	v->visit(*this);
}

dowhile_statement::dowhile_statement( boost::shared_ptr<token_attr> tok )
: statement( syntax_node_types::dowhile_statement, tok ){
}

void dowhile_statement::accept( syntax_tree_visitor* v ){
	v->visit(*this);
}


label::label( syntax_node_types type_id, boost::shared_ptr<token_attr> tok )
: node(type_id, tok){
}

jump_statement::jump_statement( boost::shared_ptr<token_attr> tok )
: statement( syntax_node_types::jump_statement, tok ), code( jump_mode::none ){
}

void jump_statement::accept( syntax_tree_visitor* v ){
	v->visit(*this);
}

switch_statement::switch_statement( boost::shared_ptr<token_attr> tok )
: statement( syntax_node_types::switch_statement, tok ){
}

void switch_statement::accept( syntax_tree_visitor* v ){
	v->visit(*this);
}

case_label::case_label( boost::shared_ptr<token_attr> tok )
: label( syntax_node_types::case_label, tok ){
}

void case_label::accept( syntax_tree_visitor* v ){
	v->visit(*this);
}

ident_label::ident_label( boost::shared_ptr<token_attr> tok )
: label( syntax_node_types::ident_label, tok ){
}

void ident_label::accept( syntax_tree_visitor* v ){
	v->visit(*this);
}
END_NS_SASL_SYNTAX_TREE();