#ifndef SASL_COMMON_TOKEN_ATTR_H
#define SASL_COMMON_TOKEN_ATTR_H

#include <sasl/include/common/common_fwd.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/make_shared.hpp>
#include <boost/smart_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

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

	static boost::shared_ptr<token_attr> from_string( const std::string& str ){
		return boost::make_shared<token_attr>(str.begin(), str.end());
	}
};

END_NS_SASL_COMMON()

#endif
