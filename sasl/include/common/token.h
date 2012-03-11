#ifndef SASL_COMMON_token_H
#define SASL_COMMON_token_H

#include <sasl/include/common/common_fwd.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

#include <string>

BEGIN_NS_SASL_COMMON();

struct code_span
{
	code_span();
	code_span( size_t line_beg, size_t col_beg, size_t line_end, size_t col_end );
	code_span( size_t line_beg, size_t col_beg, size_t length );
	void set( size_t line_beg, size_t col_beg, size_t line_end, size_t col_end );
	void set( size_t line_beg, size_t col_beg, size_t length );
	size_t line_beg, col_beg;
	size_t line_end, col_end;
	static code_span merge( code_span const& s0, code_span const& s1 );
};

class fname_t
{
public:
	fname_t();
	fname_t( std::string const& );
	std::string const& str();
private:
	boost::shared_ptr<std::string> fname;
	static std::string null_name;
};

struct token_t{
	token_t();
	token_t( const token_t& rhs );
	template< typename IteratorT >
	token_t( const IteratorT& first, const IteratorT& last )
		: str(first, last), id(0) 
	{
	}
	token_t& operator = ( const token_t& rhs);

	boost::shared_ptr<token_t> make_copy() const;

	static boost::shared_ptr<token_t> null();
	static boost::shared_ptr<token_t> from_string( const std::string& str );
	static boost::shared_ptr<token_t> make( size_t id, std::string const& str, size_t line, size_t col, std::string const& fname );

	size_t		id;
	std::string str;
	code_span	span;
	fname_t		file_name;

};

END_NS_SASL_COMMON()

#endif
