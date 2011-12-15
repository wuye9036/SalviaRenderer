#include <sasl/include/syntax_tree/statement.h>
#include <sasl/include/syntax_tree/visitor.h>
#include <sasl/include/common/token.h>

BEGIN_NS_SASL_SYNTAX_TREE();

statement::statement( node_ids nodetype, boost::shared_ptr<token_t> tok )
: node( nodetype, tok ){}

labeled_statement::labeled_statement( boost::shared_ptr<token_t> tok )
	: statement( node_ids::labeled_statement, tok ){ }

boost::shared_ptr<struct label> labeled_statement::pop_label()
{
	assert( !labels.empty() );
	boost::shared_ptr<struct label> ret = labels.back();
	labels.pop_back();
	return ret;
}

void labeled_statement::push_label( boost::shared_ptr<label> lbl )
{
	this->labels.push_back( lbl );
}

SASL_SYNTAX_NODE_ACCEPT_METHOD_IMPL( labeled_statement );

declaration_statement::declaration_statement( boost::shared_ptr<token_t> tok )
	: statement( node_ids::declaration_statement, tok ){ }

SASL_SYNTAX_NODE_ACCEPT_METHOD_IMPL( declaration_statement );

expression_statement::expression_statement( boost::shared_ptr<token_t> tok )
: statement( node_ids::expression_statement, tok ){
}

SASL_SYNTAX_NODE_ACCEPT_METHOD_IMPL( expression_statement );

SASL_SYNTAX_NODE_ACCEPT_METHOD_IMPL( compound_statement );

compound_statement::compound_statement( boost::shared_ptr<token_t> tok )
: statement( node_ids::compound_statement, tok ){
}


if_statement::if_statement( boost::shared_ptr<token_t> tok )
: statement( node_ids::if_statement, tok ){
}

SASL_SYNTAX_NODE_ACCEPT_METHOD_IMPL( if_statement );

while_statement::while_statement( boost::shared_ptr<token_t> tok )
: statement( node_ids::while_statement, tok ) {
}

SASL_SYNTAX_NODE_ACCEPT_METHOD_IMPL( while_statement );

dowhile_statement::dowhile_statement( boost::shared_ptr<token_t> tok )
: statement( node_ids::dowhile_statement, tok ){
}

SASL_SYNTAX_NODE_ACCEPT_METHOD_IMPL( dowhile_statement );

label::label( node_ids type_id, boost::shared_ptr<token_t> tok )
: node(type_id, tok){
}

jump_statement::jump_statement( boost::shared_ptr<token_t> tok )
: statement( node_ids::jump_statement, tok ), code( jump_mode::none ){
}

SASL_SYNTAX_NODE_ACCEPT_METHOD_IMPL( jump_statement );

switch_statement::switch_statement( boost::shared_ptr<token_t> tok )
: statement( node_ids::switch_statement, tok ){
}

SASL_SYNTAX_NODE_ACCEPT_METHOD_IMPL( switch_statement );

case_label::case_label( boost::shared_ptr<token_t> tok )
: label( node_ids::case_label, tok ){
}

SASL_SYNTAX_NODE_ACCEPT_METHOD_IMPL( case_label );

ident_label::ident_label( boost::shared_ptr<token_t> tok )
: label( node_ids::ident_label, tok ){
}

SASL_SYNTAX_NODE_ACCEPT_METHOD_IMPL( ident_label );

for_statement::for_statement( boost::shared_ptr<token_t> tok )
: statement( node_ids::for_statement, tok ){}

SASL_SYNTAX_NODE_ACCEPT_METHOD_IMPL( for_statement );

END_NS_SASL_SYNTAX_TREE();