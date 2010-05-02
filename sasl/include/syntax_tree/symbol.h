#ifndef SASL_SYNTAX_TREE_SYMBOL_H
#define SASL_SYNTAX_TREE_SYMBOL_H

#include "syntax_tree_fwd.h"
#include <boost/pointer_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/tr1/unordered_map.hpp>
#include <boost/tr1/type_traits.hpp>
#include <string>

BEGIN_NS_SASL_SYNTAX_TREE()

struct node;
class symbol_info;

class symbol{
public:
	static boost::shared_ptr<symbol> create( boost::shared_ptr<struct node> correspond_node );
	boost::shared_ptr<symbol> find_this(const std::string& s);
	boost::shared_ptr<symbol> find_all(const std::string& s);
	boost::shared_ptr<symbol> add_child(const std::string& s, boost::shared_ptr<node> pnode);

	boost::shared_ptr<symbol_info> symbol_info( const std::string& clsname );
	void symbol_info( boost::shared_ptr<class symbol_info> syminfo );

	template <typename T> boost::shared_ptr<T> symbol_info(){
		BOOST_STATIC_ASSERT( std::tr1::is_base_of<struct symbol_info, T>::value );
		static T instance;
		return boost::shared_polymorphic_downcast<T>(instance.class_name());
	}
	template <typename T> void symbol_info( boost::shared_ptr<T> syminfo ){
		BOOST_STATIC_ASSERT( std::tr1::is_base_of<class symbol_info, T>::value );
		symbol_info( boost::shared_polymorphic_cast<T>(syminfo) );
	}
	template <typename T> boost::shared_ptr<T> get_or_create_symbol_info(){
		BOOST_STATIC_ASSERT( std::tr1::is_base_of<class symbol_info, T>::value );
		boost::shared_ptr<T> ret = symbol_info<T>();
		if ( !ret ){
			ret.reset( new T() );
			symbol_info( ret );
		}
		return ret;
	}

	boost::shared_ptr<struct node> node();

private:
	static boost::shared_ptr<symbol> create( boost::shared_ptr<symbol> parent, boost::shared_ptr<struct node> correspond_node );
	symbol(boost::shared_ptr<symbol> parent, boost::shared_ptr<struct node> correspond_node);
	typedef std::vector< boost::shared_ptr<class symbol_info> > symbol_infos_t;
	typedef std::tr1::unordered_map< std::string, boost::shared_ptr<symbol> > children_t;
	typedef children_t::iterator children_iterator_t;

	boost::weak_ptr<struct node> correspond_node;
	boost::weak_ptr<symbol> parent;
	boost::weak_ptr<symbol> selfptr;
	children_t children;
	symbol_infos_t syminfos;
};

END_NS_SASL_SYNTAX_TREE()

#endif