#include <sasl/include/syntax_tree/symbol_info.h>
#include <sasl/include/syntax_tree/symbol.h>
#include <sasl/include/syntax_tree/node.h>

using namespace std;
using namespace boost;

BEGIN_NS_SASL_SYNTAX_TREE()

boost::shared_ptr<symbol> symbol::create( boost::shared_ptr<struct node> correspond_node ){
	return create( boost::shared_ptr<symbol>(), correspond_node );
}

boost::shared_ptr<symbol> symbol::create( boost::shared_ptr<symbol> parent, boost::shared_ptr<struct node> correspond_node ){
	boost::shared_ptr<symbol> ret( new symbol( parent, correspond_node ) );
	ret->selfptr = ret;
	return ret;
}

symbol::symbol( boost::shared_ptr<symbol> parent,
			   boost::shared_ptr<struct node> correspond_node )
			   :parent(parent),
			   correspond_node(correspond_node)
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
	return parent.lock()->find_all(s);
}

boost::shared_ptr<symbol> symbol::add_child( const std::string& s, boost::shared_ptr<struct node> pnode )
{
	children_iterator_t ret_it = children.find(s);
	if ( ret_it != children.end() ){
		return boost::shared_ptr<symbol>();
	}
	boost::shared_ptr<symbol> ret = create( selfptr.lock(), node() );
	children.insert( std::make_pair( s, ret ) );
	return ret;
}

boost::shared_ptr<class symbol_info> symbol::symbol_info( const std::string& clsname ){
	for ( symbol_infos_t::iterator it = syminfos.begin(); it != syminfos.end(); ++it ){
		if ( (*it)->class_name() == clsname ){
			return *it;
		}
	}
	return boost::shared_ptr<class symbol_info>();
}

void symbol::symbol_info( boost::shared_ptr<class symbol_info> syminfo ){
	if ( symbol_info(syminfo->class_name()) ){
		return;
	}
	syminfos.push_back( syminfo );
}

boost::shared_ptr<node> symbol::node()
{
	return correspond_node.lock();
}

END_NS_SASL_SYNTAX_TREE()