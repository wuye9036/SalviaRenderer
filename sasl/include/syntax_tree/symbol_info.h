#ifndef SASL_SYNTAX_TREE_SYMBOL_INFO_H
#define SASL_SYNTAX_TREE_SYMBOL_INFO_H

#include <sasl/include/syntax_tree/syntax_tree_fwd.h>
#include <boost/tr1/type_traits.hpp>
#include <string>

BEGIN_NS_SASL_SYNTAX_TREE()

class symbol_info{
public:
	const std::string& class_name(){
		return clsname;
	}
protected:
	symbol_info( std::string& cls ):
		 clsname(cls)
	{}
	std::string clsname;
};

template <typename T> boost::shared_ptr<T> extract_symbol_info( boost::shared_ptr<node> pnode ){
	BOOST_STATIC_ASSERT( std::tr1::is_base_of<symbol_info, T>::value );
	return pnode->symbol()->symbol_info<T>();
}

template <typename T> boost::shared_ptr<T> get_or_create_symbol_info( boost::shared_ptr<node> pnode ){
	BOOST_STATIC_ASSERT( std::tr1::is_base_of<symbol_info, T>::value );
	return pnode->symbol()->get_or_create_symbol_info<T>();
}

END_NS_SASL_SYNTAX_TREE()

#endif