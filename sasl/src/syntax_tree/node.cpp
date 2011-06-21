#include <sasl/include/syntax_tree/node.h>
#include <sasl/include/semantic/semantic_info.h>

using namespace boost;

BEGIN_NS_SASL_SYNTAX_TREE();

using ::sasl::semantic::semantic_info;

node::node(node_ids tid, shared_ptr<token_t> tok )
: type_id(tid), tok(tok)
{
	// DO NOTHING
}

boost::shared_ptr<node> node::handle() const{
	return selfptr.lock();
}

boost::shared_ptr<class ::sasl::semantic::symbol> node::symbol() const{
	return sym.lock();
}

void node::symbol( boost::shared_ptr<class ::sasl::semantic::symbol> sym ){
	this->sym = sym;
}

boost::shared_ptr<class semantic_info> node::semantic_info() const {
	return seminfo;
}

void node::semantic_info( boost::shared_ptr<class ::sasl::semantic::semantic_info> si ) const{
	const_cast<node*>(this)->seminfo = si;
}

boost::shared_ptr<token_t> node::token() const{
	return tok;
}

node_ids node::node_class() const{
	return type_id;
}

node::~node(){
	// DO NOTHING
}

const ::std::vector< ::boost::shared_ptr< node > >& node::additionals() const{
	return adds;
}

::std::vector< ::boost::shared_ptr< node > >& node::additionals(){
	return adds;
}

END_NS_SASL_SYNTAX_TREE();