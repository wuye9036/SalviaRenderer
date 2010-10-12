#include <sasl/include/semantic/symbol.h>

#include <sasl/include/syntax_tree/node.h>

#include <eflib/include/disable_warnings.h>
#include <boost/lexical_cast.hpp>
#include <boost/thread/mutex.hpp>
#include <eflib/include/enable_warnings.h>
#include <algorithm>

using namespace std;

BEGIN_NS_SASL_SEMANTIC();

static boost::shared_ptr<symbol> nullsym;
static vector< boost::shared_ptr<symbol> > empty_syms;

boost::shared_ptr<symbol> symbol::create_root( boost::shared_ptr<struct node> root_node ){
	return create( boost::shared_ptr<symbol>(), root_node, std::string("") );
}

boost::shared_ptr<symbol> symbol::create(
	boost::shared_ptr<symbol> parent,
	boost::shared_ptr<struct node> correspond_node,
	const std::string& mangled )
{
	boost::shared_ptr<symbol> ret( new symbol( parent, correspond_node, mangled ) );
	ret->selfptr = ret;
	correspond_node->symbol( ret );
	return ret;
}

symbol::symbol( boost::shared_ptr<symbol> parent,
			   boost::shared_ptr<struct node> correspond_node,
			   const string& mangled
			   )
			   :this_parent(parent),
			   correspond_node(correspond_node),
			   umgl_name( mangled ),
			   mgl_name( mangled )
{
}

boost::shared_ptr<symbol> symbol::find_this( const std::string& mangled ) const
{
	children_t::const_iterator ret_it = children.find(mangled);
	if (ret_it == children.end()){
		return nullsym;
	} else {
		return ret_it->second;
	}
}

boost::shared_ptr<symbol> symbol::find( const std::string& mangled ) const
{
	boost::shared_ptr<symbol> ret = find_this(mangled);
	if (ret) {	return ret; }
	if ( !parent() ) { return nullsym;	}
	return parent()->find(mangled);
}

const std::vector<const ::std::string>& symbol::get_overloads( const ::std::string& unmangled_name ) const
{
	overload_table_t::const_iterator found_it = overloads.find( unmangled_name );
	if ( found_it == overloads.end() ){
		return null_mt;
	}
	return found_it->second;
}

std::vector< boost::shared_ptr<symbol> > symbol::find_overloads( const ::std::string& unmangled ) const{
	vector< boost::shared_ptr<symbol> > ret;
	const std::vector<const ::std::string>& name_of_ret = get_overloads( unmangled );
	if ( !name_of_ret.empty() ) {
		for( size_t i_name = 0; i_name < name_of_ret.size(); ++i_name ){
			ret.push_back( find( name_of_ret[i_name] ) ); 
		}
		return ret;
	}
	if ( !parent() ) { return empty_syms; }
	return parent()->find_overloads( unmangled );
}

boost::shared_ptr<symbol> symbol::add_child( const std::string& mangled, boost::shared_ptr<struct node> child_node )
{
	children_iterator_t ret_it = children.find(mangled);
	if ( ret_it != children.end() ){
		return boost::shared_ptr<symbol>();
	}
	boost::shared_ptr<symbol> ret = create( selfptr.lock(), child_node, mangled );
	children.insert( std::make_pair( mangled, ret ) );
	return ret;
}

boost::shared_ptr<symbol> symbol::add_overloaded_child(
	const std::string& unmangled,
	const std::string& mangled,
	boost::shared_ptr<struct node> child_node
	)
{
	boost::shared_ptr<symbol> added_sym = add_child( mangled, child_node );
	if ( added_sym ){
		if( overloads.count( unmangled ) == 0 ){
			overloads[ unmangled ] = std::vector< const ::std::string>();
		}
		overloads[ unmangled ].push_back( mangled );
		added_sym->umgl_name = unmangled;
	}
	return added_sym;
}

void symbol::remove_child( const std::string& mangled ){
	children_iterator_t ret_it = children.find( mangled );
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

	//remove mangled item from overloaded items table.
	if ( overloads.count( rmsym->unmangled_name() ) > 0){
		overload_table_t::mapped_type& mt = overloads[umgl_name];
		overload_table_t::mapped_type::iterator mt_it
			= ::std::find( mt.begin(), mt.end(), mgl_name );
		mt.erase( mt_it );
		if ( mt.empty() ) { overloads.erase( umgl_name ); }
	}
}

void symbol::remove(){
	if ( parent() ){
		parent()->remove_child( mangled_name() );
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

const std::string& symbol::mangled_name() const{
	return mgl_name;
}

const std::string& symbol::unmangled_name() const{
	return umgl_name;
}

void symbol::add_mangling( const std::string& mangled ){
	mgl_name = mangled;

	if( !parent() ){
		return;
	}

	// add new name and remove old name
	this_parent.lock()->children.insert( make_pair( mangled, node()->symbol() ) );
	this_parent.lock()->children.erase( umgl_name );

	// add to overloaded items table
	if ( this_parent.lock()->get_overloads(umgl_name).empty() ){
		this_parent.lock()->overloads[umgl_name] = overload_table_t::mapped_type();
	}
	this_parent.lock()->overloads[umgl_name].push_back( mangled );
}

int anonymous_name_count = 0;
boost::mutex mtx;
std::string anonymous_name(){
	boost::mutex::scoped_lock locker(mtx);
	return std::string("__sasl__anonymous__name__66EE3DF2AC1746aaA963B98C7F1985E1_") + boost::lexical_cast<std::string>(++anonymous_name_count);
}

boost::shared_ptr<symbol> symbol::add_anonymous_child( boost::shared_ptr<struct node> child_node ){
	return add_child( anonymous_name(), child_node );
}

END_NS_SASL_SEMANTIC();