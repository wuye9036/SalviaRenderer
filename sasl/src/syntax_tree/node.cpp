#include <sasl/include/syntax_tree/node.h>

using namespace boost;

BEGIN_NS_SASL_SYNTAX_TREE();

node::node(syntax_node_types tid, shared_ptr<token_attr> tok )
: type_id(tid), tok(tok)
{
	// DO NOTHING
}

boost::shared_ptr<class symbol> node::symbol(){
	return sym.lock();
}

boost::shared_ptr<token_attr> node::token(){
	return tok;
}

syntax_node_types node::node_class(){
	return type_id;
}

node::~node(){
	// DO NOTHING
}


END_NS_SASL_SYNTAX_TREE();