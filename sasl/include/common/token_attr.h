#ifndef SASL_COMMON_TOKEN_ATTR_H
#define SASL_COMMON_TOKEN_ATTR_H

#include <sasl/include/common/common_fwd.h>
#include <boost/smart_ptr.hpp>
#include <string>

BEGIN_NS_SASL_COMMON()

struct token_attr{
	token_attr()
		: file_name("undefined"), column(0), line(0), str("UNINITIALIZED_VALUE"){}
	token_attr( const token_attr& rhs )
		: file_name( rhs.file_name ), column(rhs.column), line(rhs.line), str(rhs.str){}
	template< typename IteratorT > token_attr( const IteratorT& first, const IteratorT& last )
		: file_name("undefined"), column(0), line(0), str(first, last) {}

	boost::shared_ptr<token_attr> make_copy() const{
		return boost::shared_ptr<token_attr>( new token_attr(*this) );
	}

	token_attr& operator = ( const token_attr& rhs){
		file_name = rhs.file_name;
		column = rhs.column;
		line = rhs.line;
		str = rhs.str;	
		return *this;
	}

	std::string str;
	std::size_t line;
	std::size_t column;
	std::string file_name;
	
	static boost::shared_ptr<token_attr> null(){
		return boost::shared_ptr<token_attr>();
	}
};

END_NS_SASL_COMMON()

#endif