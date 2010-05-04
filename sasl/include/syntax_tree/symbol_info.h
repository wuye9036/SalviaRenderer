#ifndef SASL_SYNTAX_TREE_SYMBOL_INFO_H
#define SASL_SYNTAX_TREE_SYMBOL_INFO_H

#include <sasl/include/syntax_tree/syntax_tree_fwd.h>
#include <boost/tr1/type_traits.hpp>
#include <boost/shared_ptr.hpp>
#include <string>

BEGIN_NS_SASL_SYNTAX_TREE()

struct node;

class symbol_info{
public:
	const std::string& class_name(){
		return clsname;
	}
protected:
	symbol_info( const std::string& cls ):
		 clsname(cls)
	{}
	std::string clsname;
};

template <typename SymbolInfoT, typename NodeU> boost::shared_ptr<SymbolInfoT> extract_symbol_info( boost::shared_ptr<NodeU> pnode ){
	BOOST_STATIC_ASSERT( std::tr1::is_base_of<symbol_info, T>::value );
	BOOST_STATIC_ASSERT( std::tr1::is_base_of<node, NodeU>::value );
	return pnode->symbol()->symbol_info<SymbolInfoT>();
}

template <typename SymbolInfoT, typename NodeU> boost::shared_ptr<SymbolInfoT> extract_symbol_info( NodeU& nd ){
	BOOST_STATIC_ASSERT( std::tr1::is_base_of<symbol_info, T>::value );
	BOOST_STATIC_ASSERT( std::tr1::is_base_of<node, NodeU>::value );
	return nd.symbol()->symbol_info<SymbolInfoT>();
}

template <typename SymbolInfoT, typename NodeU> boost::shared_ptr<SymbolInfoT> get_or_create_symbol_info( boost::shared_ptr<NodeU> pnode ){
	BOOST_STATIC_ASSERT( std::tr1::is_base_of<symbol_info, T>::value );
	BOOST_STATIC_ASSERT( std::tr1::is_base_of<node, NodeU>::value );
	return pnode->symbol()->get_or_create_symbol_info<SymbolInfoT>();
}

template <typename SymbolInfoT, typename NodeU> boost::shared_ptr<SymbolInfoT> get_or_create_symbol_info( NodeU& nd ){
	BOOST_STATIC_ASSERT( std::tr1::is_base_of<symbol_info, T>::value );
	BOOST_STATIC_ASSERT( std::tr1::is_base_of<node, NodeU>::value );
	return nd.symbol()->get_or_create_symbol_info<SymbolInfoT>();
}

END_NS_SASL_SYNTAX_TREE()

#endif