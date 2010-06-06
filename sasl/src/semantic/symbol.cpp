#include <sasl/include/semantic/symbol.h>
#include <sasl/include/syntax_tree/node.h>
#include <algorithm>

using namespace std;

BEGIN_NS_SASL_SEMANTIC();

boost::shared_ptr<symbol> symbol::create_root( boost::shared_ptr<struct node> root_node ){
	return create( boost::shared_ptr<symbol>(), root_node, std::string("") );
}

boost::shared_ptr<symbol> symbol::create( boost::shared_ptr<symbol> parent, boost::shared_ptr<struct node> correspond_node, const std::string& name ){
	boost::shared_ptr<symbol> ret( new symbol( parent, correspond_node, name ) );
	ret->selfptr = ret;
	correspond_node->symbol( ret );
	return ret;
}

symbol::symbol( boost::shared_ptr<symbol> parent,
			   boost::shared_ptr<struct node> correspond_node,
			   const string& name
			   )
			   :this_parent(parent),
			   correspond_node(correspond_node),
			   symname( name )
{
}

boost::shared_ptr<symbol> symbol::find_mangled_this( const std::string& s ) const
{
	children_t::const_iterator ret_it = children.find(s);
	if (ret_it == children.end()){
		return boost::shared_ptr<symbol>();
	} else {
		return ret_it->second;
	}
}

boost::shared_ptr<symbol> symbol::find_mangled_all( const std::string& s ) const
{
	boost::shared_ptr<symbol> this_ret = find_mangled_this(s);
	if (this_ret) {	return this_ret; }
	if ( !parent() ) { return boost::shared_ptr<symbol>();	}
	return parent()->find_mangled_all(s);
}

const std::vector<const ::std::string>& symbol::find_mangles_this( const ::std::string& unmangled_name ) const
{
	mangling_table_t::const_iterator found_it = mangles.find( unmangled_name );
	if ( found_it == mangles.end() ){
		return null_mt;
	}
	return found_it->second;
}

const std::vector<const ::std::string>& symbol::find_mangles_all( const ::std::string& unmangled_name ) const{
	const std::vector<const ::std::string>& this_ret = find_mangles_this( unmangled_name );
	if ( !this_ret.empty() ) { return this_ret; }
	if ( !parent() ) { return null_mt; }
	return parent()->find_mangles_all( unmangled_name );
}

bool symbol::contains_symbol_this( const ::std::string& str ) const{
	return (!find_mangles_this(str).empty()) || find_mangled_this(str);
}

bool symbol::contains_symbol_all( const ::std::string& str ) const{
	return (!find_mangles_all(str).empty()) || find_mangled_all(str);
}

boost::shared_ptr<symbol> symbol::add_child( const std::string& s, boost::shared_ptr<struct node> child_node )
{
	children_iterator_t ret_it = children.find(s);
	if ( ret_it != children.end() ){
		return boost::shared_ptr<symbol>();
	}
	boost::shared_ptr<symbol> ret = create( selfptr.lock(), child_node, s );
	children.insert( std::make_pair( s, ret ) );
	return ret;
}

boost::shared_ptr<symbol> symbol::add_mangled_child(
	const std::string& unmangled_name,
	const std::string& mangled_name,
	boost::shared_ptr<struct node> child_node
	)
{
	boost::shared_ptr<symbol> added_sym = add_child( mangled_name, child_node );
	if ( added_sym ){
		const std::vector<const ::std::string>& mangled_names = find_mangles_this( unmangled_name );
		if ( mangled_names.empty() ){
			mangles[unmangled_name] = std::vector< const ::std::string>();
		}
		mangles[unmangled_name].push_back( mangled_name );
		added_sym->unmangled_name = unmangled_name;
	}
	return added_sym;
}

void symbol::remove_child( const std::string& s ){
	children_iterator_t ret_it = children.find(s);
	if ( ret_it == children.end() ){
		return;
	}
	// remove symbol from corresponding node.
	boost::shared_ptr<symbol> rmsym = ret_it->second;
	if( rmsym->node() ){
		rmsym->node()->symbol( boost::shared_ptr<symbol>() );
	}
	// remove itself.
	children.erase( ret_it );

	//remove mangled_name from mangling table.
	if ( mangles.count( unmangled_name ) > 0){
		mangling_table_t::mapped_type& mt = mangles[unmangled_name];
		mangling_table_t::mapped_type::const_iterator mt_it
			= ::std::find( mt.begin(), mt.end(), symname );
		mt.erase( mt_it );
		if ( mt.empty() ) { mangles.erase( unmangled_name ); }
	}
}

void symbol::remove_from_tree(){
	if ( parent() ){
		parent()->remove_child( name() );
	}
}

boost::shared_ptr<symbol> symbol::parent() const{
	return this_parent.lock();
}

boost::shared_ptr<node> symbol::node() const
{
	return correspond_node.lock();
}

void symbol::relink( boost::shared_ptr<struct node> n ){
	correspond_node = n;
}

const std::string& symbol::name() const{
	return symname;
}

END_NS_SASL_SEMANTIC();