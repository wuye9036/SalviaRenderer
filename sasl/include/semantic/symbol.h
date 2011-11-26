#ifndef SASL_SEMANTIC_SYMBOL_H
#define SASL_SEMANTIC_SYMBOL_H

#include <sasl/include/semantic/semantic_forward.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/pointer_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/static_assert.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/tr1/unordered_map.hpp>
#include <boost/tr1/type_traits.hpp>
#include <eflib/include/platform/boost_end.h>

#include <string>
#include <vector>

namespace sasl{
	namespace syntax_tree{
		struct node;
		struct expression;
		struct function_type;
		struct tynode;
	}
}

BEGIN_NS_SASL_SEMANTIC();

using sasl::syntax_tree::function_type;
using sasl::syntax_tree::node;
using sasl::syntax_tree::tynode;
class tecov_t;

//////////////////////////////////////////////////////////////////////////
/*
	In sasl, symbol can assigned to any syntax node. But in fact, only some
	sorts of node make senses, such as variable declaration, function and
	struct declaration/definition, type re-definition, statement.

	Every symbol has three names.
	* THE FIRST NAME is "unmangled name", briefly "name". It means
	the literal name of syntax node. For a example, a variable name or a label.
	* THE SECOND NAME is "mangled name". It is useful for function overloading.
	There is a document describing its details.
	* THE THIRD NAME is "full path". It is a compile unit unique name or
	a target unique name, even a global unique name. Some external interface
	may use it.
*/
//////////////////////////////////////////////////////////////////////////
class symbol{
public:
	typedef std::vector< boost::shared_ptr<symbol> > overloads_t;
	typedef overloads_t::iterator overloads_iterator_t;

	static boost::shared_ptr<symbol> create_root( boost::shared_ptr<struct node> root_node = boost::shared_ptr<struct node>() );

	boost::shared_ptr<symbol> find( const std::string& name ) const;
	boost::shared_ptr<symbol> find_this(const std::string& mangled) const;

	std::vector< boost::shared_ptr<symbol> > find_overloads( const std::string& name ) const;
	std::vector< boost::shared_ptr<symbol> > find_overloads(
		const std::string& name,
		boost::shared_ptr<tecov_t> conv,
		std::vector< boost::shared_ptr< ::sasl::syntax_tree::expression > > args
		) const;
	int count( std::string name ) const;

	boost::shared_ptr<symbol> add_child(const std::string& mangled, boost::shared_ptr<node> child_node);
	
	enum unique_name_types{
		unique_in_unit,	/* Generate unique name in unit.*/
		unique_in_module,	/* Generate unique name in module. */
		unnamed_struct
	};
	static std::string unique_name( unique_name_types unique_type = unique_in_unit );

	boost::shared_ptr<symbol> add_anonymous_child( boost::shared_ptr<node> child_node );

	boost::shared_ptr<symbol> add_function_begin( boost::shared_ptr<function_type> child_fn );
	bool add_function_end( boost::shared_ptr<symbol> sym );

	void remove_child( const std::string& mangled );
	void remove();

	boost::shared_ptr<symbol> parent() const;

	boost::shared_ptr<struct node> node() const;
	void relink( boost::shared_ptr<struct node> n );

	const std::string& unmangled_name() const;
	const std::string& mangled_name() const;

	// std::string fullpath();
private:
	static boost::shared_ptr<symbol> create(
		boost::shared_ptr<symbol> parent,
		boost::shared_ptr<struct node> correspond_node,
		const std::string& mangled
		);
	symbol(
		boost::shared_ptr<symbol> parent,
		boost::shared_ptr<struct node> correspond_node,
		const std::string& mangled
		);

	const std::vector< ::std::string >& get_overloads( const ::std::string& umnalged ) const;

	typedef std::tr1::unordered_map< std::string, boost::shared_ptr<symbol> > children_t;
	typedef std::tr1::unordered_map< std::string, ::std::vector< ::std::string > > overload_table_t;
	typedef children_t::iterator children_iterator_t;

	boost::shared_ptr<struct node> correspond_node;
	boost::weak_ptr<symbol> this_parent;
	boost::weak_ptr<symbol> selfptr;

	children_t children;
	overload_table_t overloads;
	::std::vector< ::std::string > null_mt;
	// name
	std::string mgl_name;
	std::string umgl_name;
};

END_NS_SASL_SEMANTIC()

#endif
