#include <sasl/include/syntax_tree/node.h>
#include <sasl/include/semantic/semantic_info.h>

using namespace boost;

BEGIN_NS_SASL_SYNTAX_TREE();

using ::sasl::semantic::semantic_info_collection;

node::node(syntax_node_types tid, shared_ptr<token_attr> tok )
: type_id(tid), tok(tok)
{
	// DO NOTHING
}

boost::shared_ptr<node> node::handle() const{
	return selfptr.lock();
}

boost::shared_ptr<class ::sasl::semantic::symbol> node::symbol() const{
	return sym;
}

void node::symbol( boost::shared_ptr<class ::sasl::semantic::symbol> sym ){
	this->sym = sym;
}

boost::shared_ptr<class ::sasl::semantic::semantic_info_collection> node::semantic_infos() const {
	if ( !seminfos ){
		seminfos.reset( new semantic_info_collection() );
	}
	return seminfos;
}

boost::shared_ptr<token_attr> node::token() const{
	return tok;
}

syntax_node_types node::node_class() const{
	return type_id;
}

node::~node(){
	// DO NOTHING
}


END_NS_SASL_SYNTAX_TREE();