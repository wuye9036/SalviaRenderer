#ifndef SASL_SEMANTIC_SYMBOL_INFO_H
#define SASL_SEMANTIC_SYMBOL_INFO_H

#include <sasl/include/semantic/semantic_forward.h>
#include <boost/tr1/type_traits.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/static_assert.hpp>
#include <string>

namespace sasl {
	namespace syntax_tree{
		struct node;
	}
}

BEGIN_NS_SASL_SEMANTIC();

using sasl::syntax_tree::node;

class symbol_info{
public:
	const std::string& class_name() const{
		return clsname;
	}

	virtual ~symbol_info(){
	}
protected:
	symbol_info( const std::string& cls ):
		 clsname(cls)
	{}
	std::string clsname;
};

template <typename SymbolInfoT, typename NodeU> boost::shared_ptr<SymbolInfoT> extract_symbol_info( boost::shared_ptr<NodeU> pnode ){
	BOOST_STATIC_ASSERT( (std::tr1::is_base_of<symbol_info, SymbolInfoT>::value) );
	BOOST_STATIC_ASSERT( (std::tr1::is_base_of<node, NodeU>::value) );
	return pnode->symbol()->symbol_info<SymbolInfoT>();
}

template <typename SymbolInfoT, typename NodeU> boost::shared_ptr<SymbolInfoT> extract_symbol_info( NodeU& nd ){
	BOOST_STATIC_ASSERT( (std::tr1::is_base_of<symbol_info, SymbolInfoT>::value) );
	BOOST_STATIC_ASSERT( (std::tr1::is_base_of<node, NodeU>::value) );
	return nd.symbol()->symbol_info<SymbolInfoT>();
}

template <typename SymbolInfoT, typename NodeU> boost::shared_ptr<SymbolInfoT> get_or_create_symbol_info( boost::shared_ptr<NodeU> pnode ){
	BOOST_STATIC_ASSERT( (std::tr1::is_base_of<symbol_info, SymbolInfoT>::value) );
	BOOST_STATIC_ASSERT( (std::tr1::is_base_of<node, NodeU>::value) );
	return pnode->symbol()->get_or_create_symbol_info<SymbolInfoT>();
}

template <typename SymbolInfoT, typename NodeU> boost::shared_ptr<SymbolInfoT> get_or_create_symbol_info( NodeU& nd ){
	BOOST_STATIC_ASSERT( (std::tr1::is_base_of<symbol_info, SymbolInfoT>::value) );
	BOOST_STATIC_ASSERT( (std::tr1::is_base_of<node, NodeU>::value) );
	return nd.symbol()->get_or_create_symbol_info<SymbolInfoT>();
}

END_NS_SASL_SEMANTIC();

#endif