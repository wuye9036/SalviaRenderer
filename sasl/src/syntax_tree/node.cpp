#include <sasl/include/syntax_tree/node.h>

using namespace boost;

BEGIN_NS_SASL_SYNTAX_TREE()

node::node(syntax_node_types tid, shared_ptr<token_attr> tok )
: type_id(type), tok(tok)
{
	// DO NOTHING
}

node::~node(){
	// DO NOTHING
}


END_NS_SASL_SYNTAX_TREE()