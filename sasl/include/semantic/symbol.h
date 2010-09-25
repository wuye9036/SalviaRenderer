#ifndef SASL_SEMANTIC_SYMBOL_H
#define SASL_SEMANTIC_SYMBOL_H

#include <sasl/include/semantic/semantic_forward.h>
#include <boost/pointer_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/static_assert.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/tr1/unordered_map.hpp>
#include <boost/tr1/type_traits.hpp>
#include <string>
#include <vector>

namespace sasl{
	namespace syntax_tree{
		struct node;
	}
}

BEGIN_NS_SASL_SEMANTIC();

using sasl::syntax_tree::node;

//////////////////////////////////////////////////////////////////////////
/*
	In sasl, symbol can assigned to any syntax node. But in fact, only some
	sorts of node make senses, such as variable declaration, function and
	struct declaration/definition, type re-definition, statement.

	Every symbol has three names.
	* THE FIRST NAME is "unmangled name". It means
	  the literal name of syntax node. For a example, a variable name or a label.
	* THE SECOND NAME is "mangled name". It is useful for function overloading.
	  There is a document describing its details.
	* THE THIRD NAME is "full path name". It is a compile unit unique name or
	  a target unique name, even a global unique name. Some external interface
	  may use it.
*/
//////////////////////////////////////////////////////////////////////////
class symbol{
public:
	static boost::shared_ptr<symbol> create_root( boost::shared_ptr<struct node> root_node );

	boost::shared_ptr<symbol> find_mangled_this(const std::string& mangled_name) const;
	boost::shared_ptr<symbol> find_mangled_all(const std::string& mangled_name) const;
	const std::vector<const ::std::string>& find_mangles_this( const ::std::string& unmangled_name ) const;
	const std::vector<const ::std::string>& find_mangles_all( const ::std::string& unmangled_name ) const;

	bool contains_symbol_this( const ::std::string& str ) const;
	bool contains_symbol_all( const ::std::string& str ) const;

	boost::shared_ptr<symbol> add_child(const std::string& s, boost::shared_ptr<node> child_node);
	boost::shared_ptr<symbol> add_mangled_child(
		const std::string& unmangled_name,
		const std::string& mangled_name,
		boost::shared_ptr<struct node> child_node
		);
	void remove_child( const std::string& mangled_name );
	
	void remove_from_tree();
	boost::shared_ptr<symbol> parent() const;

	boost::shared_ptr<struct node> node() const;
	void relink( boost::shared_ptr<struct node> n );

	const std::string& name() const;
	// modify a name with full path.
	// it can generate unique name in an module.
	// e.g.:
	//
	//	/* module x_mod; */
	//	void foo(){
	//		{
	//			label: ====> full path of it is @x_mod$$0$label <- label name
	//				;                              ^   ^ $0 is anonymous block 0
	//		}                                     module name
	//	}
	std::string get_fullpath_name( const std::string& );
	std::string fullpath();
private:
	static boost::shared_ptr<symbol> create(
		boost::shared_ptr<symbol> parent,
		boost::shared_ptr<struct node> correspond_node,
		const std::string& name
		);
	symbol(
		boost::shared_ptr<symbol> parent,
		boost::shared_ptr<struct node> correspond_node,
		const std::string& name
		);

	typedef std::tr1::unordered_map< std::string, boost::shared_ptr<symbol> > children_t;
	typedef std::tr1::unordered_map< std::string, ::std::vector< const ::std::string > > mangling_table_t;
	typedef children_t::iterator children_iterator_t;

	boost::weak_ptr<struct node> correspond_node;
	boost::weak_ptr<symbol> this_parent;
	boost::weak_ptr<symbol> selfptr;

	children_t children;
	mangling_table_t mangles;
	::std::vector< const ::std::string > null_mt;

	// mangled name.
	std::string symname;
	std::string unmangled_name;
};

END_NS_SASL_SEMANTIC()

#endif