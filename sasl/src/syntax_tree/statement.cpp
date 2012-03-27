#include <sasl/include/syntax_tree/statement.h>
#include <sasl/include/syntax_tree/visitor.h>
#include <sasl/include/common/token.h>

using boost::shared_ptr;

BEGIN_NS_SASL_SYNTAX_TREE();

statement::statement( node_ids nodetype, shared_ptr<token_t> const& tok_beg, shared_ptr<token_t> const& tok_end )
: node( nodetype, tok_beg, tok_end ){}

labeled_statement::labeled_statement( shared_ptr<token_t> const& tok_beg, shared_ptr<token_t> const& tok_end )
	: statement( node_ids::labeled_statement, tok_beg, tok_end ){ }

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

declaration_statement::declaration_statement( shared_ptr<token_t> const& tok_beg, shared_ptr<token_t> const& tok_end )
	: statement( node_ids::declaration_statement, tok_beg, tok_end ){ }

SASL_SYNTAX_NODE_ACCEPT_METHOD_IMPL( declaration_statement );

expression_statement::expression_statement( shared_ptr<token_t> const& tok_beg, shared_ptr<token_t> const& tok_end )
: statement( node_ids::expression_statement, tok_beg, tok_end ){
}

SASL_SYNTAX_NODE_ACCEPT_METHOD_IMPL( expression_statement );

SASL_SYNTAX_NODE_ACCEPT_METHOD_IMPL( compound_statement );

compound_statement::compound_statement( shared_ptr<token_t> const& tok_beg, shared_ptr<token_t> const& tok_end )
: statement( node_ids::compound_statement, tok_beg, tok_end ){
}


if_statement::if_statement( shared_ptr<token_t> const& tok_beg, shared_ptr<token_t> const& tok_end )
: statement( node_ids::if_statement, tok_beg, tok_end ){
}

SASL_SYNTAX_NODE_ACCEPT_METHOD_IMPL( if_statement );

while_statement::while_statement( shared_ptr<token_t> const& tok_beg, shared_ptr<token_t> const& tok_end )
: statement( node_ids::while_statement, tok_beg, tok_end ) {
}

SASL_SYNTAX_NODE_ACCEPT_METHOD_IMPL( while_statement );

dowhile_statement::dowhile_statement( shared_ptr<token_t> const& tok_beg, shared_ptr<token_t> const& tok_end )
: statement( node_ids::dowhile_statement, tok_beg, tok_end ){
}

SASL_SYNTAX_NODE_ACCEPT_METHOD_IMPL( dowhile_statement );

label::label( node_ids type_id, shared_ptr<token_t> const& tok_beg, shared_ptr<token_t> const& tok_end )
: node(type_id, tok_beg, tok_end){
}

jump_statement::jump_statement( shared_ptr<token_t> const& tok_beg, shared_ptr<token_t> const& tok_end )
: statement( node_ids::jump_statement, tok_beg, tok_end ), code( jump_mode::none ){
}

SASL_SYNTAX_NODE_ACCEPT_METHOD_IMPL( jump_statement );

switch_statement::switch_statement( shared_ptr<token_t> const& tok_beg, shared_ptr<token_t> const& tok_end )
: statement( node_ids::switch_statement, tok_beg, tok_end ){
}

SASL_SYNTAX_NODE_ACCEPT_METHOD_IMPL( switch_statement );

case_label::case_label( shared_ptr<token_t> const& tok_beg, shared_ptr<token_t> const& tok_end )
: label( node_ids::case_label, tok_beg, tok_end ){
}

SASL_SYNTAX_NODE_ACCEPT_METHOD_IMPL( case_label );

ident_label::ident_label( shared_ptr<token_t> const& tok_beg, shared_ptr<token_t> const& tok_end )
: label( node_ids::ident_label, tok_beg, tok_end ){
}

SASL_SYNTAX_NODE_ACCEPT_METHOD_IMPL( ident_label );

for_statement::for_statement( shared_ptr<token_t> const& tok_beg, shared_ptr<token_t> const& tok_end )
: statement( node_ids::for_statement, tok_beg, tok_end ){}

SASL_SYNTAX_NODE_ACCEPT_METHOD_IMPL( for_statement );

END_NS_SASL_SYNTAX_TREE();