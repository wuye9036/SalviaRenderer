#ifndef SASL_COMMON_TOKEN_ATTR_H
#define SASL_COMMON_TOKEN_ATTR_H

#include <sasl/include/common/common_fwd.h>
#include <boost/smart_ptr.hpp>
#include <string>

BEGIN_NS_SASL_COMMON()

struct token_attr{
	typedef boost::shared_ptr<token_attr> handle_t;

	token_attr()
		: file_name("undefined"), column(0), line(0), lit("UNINITIALIZED_VALUE"){}
	token_attr( const token_attr& rhs )
		: file_name( rhs.file_name ), column(rhs.column), line(rhs.line), lit(rhs.lit){}
	template< typename IteratorT > token_attr( const IteratorT& first, const IteratorT& last )
		: file_name("undefined"), column(0), line(0), lit(first, last) {}

	token_attr::handle_t make_copy() const{
		return token_attr::handle_t( new token_attr(*this) );
	}

	token_attr& operator = ( const token_attr& rhs){
		file_name = rhs.file_name;
		column = rhs.column;
		line = rhs.line;
		lit = rhs.lit;	
		return *this;
	}

	std::string lit;
	std::size_t line;
	std::size_t column;
	std::string file_name;
};

END_NS_SASL_COMMON()

#endif