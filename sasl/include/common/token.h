#ifndef SASL_COMMON_token_H
#define SASL_COMMON_token_H

#include <sasl/include/common/common_fwd.h>

#include <eflib/include/string/ustring.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

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
	static code_span merge(code_span const& s0, code_span const& s1);
};

struct token_t{
	token_t();
	
	token_t( const token_t& rhs );

	template< typename IteratorT >
	token_t(IteratorT const& first, IteratorT const& last )
		: str(first, last), id(0), end_of_file(false)
	{
	}

	token_t& operator = ( const token_t& rhs);

	boost::shared_ptr<token_t> make_copy() const;

	static boost::shared_ptr<token_t> null();
	static boost::shared_ptr<token_t> from_string( eflib::fixed_string const& str );
	static boost::shared_ptr<token_t> make(
		size_t id, eflib::fixed_string const& str,
		size_t line, size_t col, eflib::fixed_string const& fname
		);

	size_t				id;
	eflib::fixed_string	str;
	code_span			span;
	eflib::fixed_string	file_name;
	bool				end_of_file;
};

END_NS_SASL_COMMON()

#endif
