#ifndef SASL_COMMON_token_H
#define SASL_COMMON_token_H

#include <sasl/include/common/common_fwd.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/make_shared.hpp>
#include <boost/smart_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

#include <string>

BEGIN_NS_SASL_COMMON()

struct token_t{
	token_t()
		: file_name("undefined"), column(0), line(0), str("UNINITIALIZED_VALUE"){}
	token_t( const token_t& rhs )
		: file_name( rhs.file_name ), column(rhs.column), line(rhs.line), str(rhs.str), id(rhs.id){}
	template< typename IteratorT > token_t( const IteratorT& first, const IteratorT& last )
		: file_name("undefined"), column(0), line(0), str(first, last), id(0) {}

	static boost::shared_ptr<token_t> make( size_t id, std::string const& str, size_t line, size_t col, std::string const& fname ){
		boost::shared_ptr<token_t> ret = boost::make_shared<token_t>();
		ret->id = id;
		ret->str = str;
		ret->line = line;
		ret->column = col;
		ret->file_name = fname;
		return ret;
	}

	boost::shared_ptr<token_t> make_copy() const{
		return boost::shared_ptr<token_t>( new token_t(*this) );
	}

	token_t& operator = ( const token_t& rhs){
		file_name = rhs.file_name;
		column = rhs.column;
		line = rhs.line;
		str = rhs.str;
		return *this;
	}

	size_t id;

	std::string str;
	std::size_t line;
	std::size_t column;
	std::string file_name;

	static boost::shared_ptr<token_t> null(){
		return boost::shared_ptr<token_t>();
	}

	static boost::shared_ptr<token_t> from_string( const std::string& str ){
		return boost::make_shared<token_t>(str.begin(), str.end());
	}
};

END_NS_SASL_COMMON()

#endif
