#include <sasl/include/syntax_tree/program.h>

#include <sasl/enums/syntax_node_types.h>
#include <sasl/include/syntax_tree/visitor.h>

BEGIN_NS_SASL_SYNTAX_TREE();

program::program()
	: node( syntax_node_types::program, boost::shared_ptr<token_attr>() )
{
}

void program::accept( syntax_tree_visitor* v ){
	v->visit( *this );
}

END_NS_SASL_SYNTAX_TREE();