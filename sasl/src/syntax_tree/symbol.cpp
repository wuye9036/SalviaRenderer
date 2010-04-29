#include <sasl/include/syntax_tree/node.h>
#include <sasl/include/syntax_tree/symbol_info.h>
#include <sasl/include/syntax_tree/symbol.h>

using namespace std;
using namespace boost;

BEGIN_NS_SASL_SYNTAX_TREE()

symbol::symbol( boost::shared_ptr<symbol> parent,
			   boost::shared_ptr<node> correspond_node )
			   :parent(parent),
			   correspond_node(correspond_node),
			   sym_info(  symbol_info() )
{
}

boost::shared_ptr<symbol> symbol::find_this( const std::string& s )
{
	children_iterator_t ret_it = children.find(s);
	if (ret_it == children.end()){
		return boost::shared_ptr<symbol>();
	} else {
		return ret_it->second;
	}
}

boost::shared_ptr<symbol> symbol::find_all( const std::string& s )
{
	boost::shared_ptr<symbol> this_ret = find_this(s);
	if (this_ret) {	return this_ret; }
	if (parent.expired()) { return boost::shared_ptr<symbol>();	}
	return parent->find_all(s);
}

boost::shared_ptr<symbol> symbol::add_child( const std::string& s, boost::shared_ptr<node> pnode )
{
	children_iterator_t ret_it = children.find(s);
	if ( ret_it != children.end() ){
		return boost::shared_ptr<symbol>();
	}
	boost::shared_ptr<symbol> ret( new symbol(node) );
	children.insert( std::make_pair( s, ret ) );
	return ret;
}

boost::shared_ptr<symbol_info> symbol::symbol_info()
{
	return sym_info;
}

boost::shared_ptr<node> symbol::node()
{
	return correspond_node;
}

END_NS_SASL_SYNTAX_TREE()