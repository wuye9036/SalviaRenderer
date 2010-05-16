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

namespace sasl{
	namespace syntax_tree{
		struct node;
	}
}

BEGIN_NS_SASL_SEMANTIC();

class symbol_info;
using sasl::syntax_tree::node;

class symbol{
public:
	static boost::shared_ptr<symbol> create_root( boost::shared_ptr<struct node> root_node );

	boost::shared_ptr<symbol> find_this(const std::string& s);
	boost::shared_ptr<symbol> find_all(const std::string& s);
	boost::shared_ptr<symbol> add_child(const std::string& s, boost::shared_ptr<node> child_node);
	void remove_child( const std::string& s );
	void remove_from_tree();
	boost::shared_ptr<symbol> parent();

	boost::shared_ptr<symbol_info> symbol_info( const std::string& clsname );
	void symbol_info( boost::shared_ptr<class symbol_info> syminfo );

	template <typename T> boost::shared_ptr<T> symbol_info(){
		BOOST_STATIC_ASSERT( (std::tr1::is_base_of<class symbol_info, T>::value) );
		static T instance;
		return boost::shared_polymorphic_downcast<T>( symbol_info(instance.class_name()) );
	}
	template <typename T> void symbol_info( boost::shared_ptr<T> syminfo ){
		BOOST_STATIC_ASSERT( (std::tr1::is_base_of<class symbol_info, T>::value) );
		symbol_info( boost::shared_polymorphic_cast<class symbol_info>(syminfo) );
	}
	template <typename T> boost::shared_ptr<T> get_or_create_symbol_info(){
		BOOST_STATIC_ASSERT( (std::tr1::is_base_of<class symbol_info, T>::value) );
		boost::shared_ptr<T> ret = symbol_info<T>();
		if ( !ret ){
			ret.reset( new T() );
			symbol_info( ret );
		}
		return ret;
	}

	boost::shared_ptr<struct node> node();
	const std::string& name() const;
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
	typedef std::vector< boost::shared_ptr<class symbol_info> > symbol_infos_t;
	typedef std::tr1::unordered_map< std::string, boost::shared_ptr<symbol> > children_t;
	typedef children_t::iterator children_iterator_t;

	boost::weak_ptr<struct node> correspond_node;
	boost::weak_ptr<symbol> this_parent;
	boost::weak_ptr<symbol> selfptr;
	children_t children;
	symbol_infos_t syminfos;
	std::string symname;
};

END_NS_SASL_SEMANTIC()

#endif