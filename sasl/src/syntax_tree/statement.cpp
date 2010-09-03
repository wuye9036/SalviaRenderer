#include <sasl/include/syntax_tree/statement.h>
#include <sasl/include/syntax_tree/visitor.h>
#include <sasl/include/common/token_attr.h>

BEGIN_NS_SASL_SYNTAX_TREE();

statement::statement( syntax_node_types nodetype, boost::shared_ptr<token_attr> tok )
	: node( nodetype, tok )
{
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



END_NS_SASL_SYNTAX_TREE();