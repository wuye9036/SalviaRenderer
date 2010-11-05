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

SASL_SYNTAX_NODE_ACCEPT_METHOD_IMPL( declaration_statement );

expression_statement::expression_statement( boost::shared_ptr<token_attr> tok )
: statement( syntax_node_types::expression_statement, tok ){
}

SASL_SYNTAX_NODE_ACCEPT_METHOD_IMPL( expression_statement );

SASL_SYNTAX_NODE_ACCEPT_METHOD_IMPL( compound_statement );

compound_statement::compound_statement( boost::shared_ptr<token_attr> tok )
: statement( syntax_node_types::compound_statement, tok ){
}


if_statement::if_statement( boost::shared_ptr<token_attr> tok )
: statement( syntax_node_types::if_statement, tok ){
}

SASL_SYNTAX_NODE_ACCEPT_METHOD_IMPL( if_statement );

while_statement::while_statement( boost::shared_ptr<token_attr> tok )
: statement( syntax_node_types::while_statement, tok ) {
}

SASL_SYNTAX_NODE_ACCEPT_METHOD_IMPL( while_statement );

dowhile_statement::dowhile_statement( boost::shared_ptr<token_attr> tok )
: statement( syntax_node_types::dowhile_statement, tok ){
}

SASL_SYNTAX_NODE_ACCEPT_METHOD_IMPL( dowhile_statement );

label::label( syntax_node_types type_id, boost::shared_ptr<token_attr> tok )
: node(type_id, tok){
}

jump_statement::jump_statement( boost::shared_ptr<token_attr> tok )
: statement( syntax_node_types::jump_statement, tok ), code( jump_mode::none ){
}

SASL_SYNTAX_NODE_ACCEPT_METHOD_IMPL( jump_statement );

switch_statement::switch_statement( boost::shared_ptr<token_attr> tok )
: statement( syntax_node_types::switch_statement, tok ){
}

SASL_SYNTAX_NODE_ACCEPT_METHOD_IMPL( switch_statement );

case_label::case_label( boost::shared_ptr<token_attr> tok )
: label( syntax_node_types::case_label, tok ){
}

SASL_SYNTAX_NODE_ACCEPT_METHOD_IMPL( case_label );

ident_label::ident_label( boost::shared_ptr<token_attr> tok )
: label( syntax_node_types::ident_label, tok ){
}

SASL_SYNTAX_NODE_ACCEPT_METHOD_IMPL( ident_label );

for_statement::for_statement( boost::shared_ptr<token_attr> tok )
: statement( syntax_node_types::for_statement, tok ){}

SASL_SYNTAX_NODE_ACCEPT_METHOD_IMPL( for_statement );

END_NS_SASL_SYNTAX_TREE();