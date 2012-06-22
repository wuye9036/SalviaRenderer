#include <sasl/include/syntax_tree/node.h>

using namespace boost;

BEGIN_NS_SASL_SYNTAX_TREE();

EFLIB_USING_SHARED_PTR(sasl::semantic, node_semantic);
EFLIB_USING_SHARED_PTR(sasl::semantic, symbol);

node::node(node_ids tid, token_t_ptr const& tok_beg, token_t_ptr const& tok_end )
: type_id(tid), tok_beg(tok_beg), tok_end(tok_end)
{
	// DO NOTHING
}

node_ptr node::as_handle() const{
	return const_cast<node*>(this)->shared_from_this();
}

token_t_ptr node::token_begin() const{
	return tok_beg;
}

token_t_ptr node::token_end() const{
	return tok_end;
}

node_ids node::node_class() const{
	return type_id;
}

node::~node(){
	// DO NOTHING
}

void node::token_range( token_t_ptr const& tok_beg, token_t_ptr const& tok_end )
{
	this->tok_beg = tok_beg;
	this->tok_end = tok_end;
}

END_NS_SASL_SYNTAX_TREE();