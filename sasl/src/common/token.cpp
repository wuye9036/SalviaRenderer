#include <sasl/include/common/token.h>

using eflib::fixed_string;
using boost::shared_ptr;
using boost::make_shared;

BEGIN_NS_SASL_COMMON();

token_t::token_t(): str("UNINITIALIZED_VALUE"), end_of_file(false)
{
}

token_t::token_t(const token_t& rhs) : file_name(rhs.file_name), span(rhs.span), str(rhs.str), id(rhs.id), end_of_file(rhs.end_of_file)
{
}

boost::shared_ptr<token_t> token_t::make(
	size_t id, eflib::fixed_string const& str,
	size_t line, size_t col, eflib::fixed_string const& fname
	)
{
	boost::shared_ptr<token_t> ret = boost::make_shared<token_t>();
	ret->id = id;
	ret->str = str;
	ret->file_name = fname;
	ret->span.set( line, col, 0 );
	ret->end_of_file = false;

	size_t cur_line = line;
	size_t cur_col = col;
	for(fixed_string::const_iterator it = str.begin(); it != str.end(); ++it)
	{
		char ch = *it;
		if( ch == '\n' ){
			++cur_line;
			cur_col = 1;
		} else {
			++cur_col;
		}
	}

	ret->span.col_end = cur_col;
	ret->span.line_end = cur_line;

	return ret;
}

shared_ptr<token_t> token_t::make_copy() const
{
	return shared_ptr<token_t>( new token_t(*this) );
}

token_t& token_t::operator=( const token_t& rhs )
{
	file_name = rhs.file_name;
	span = rhs.span;
	str = rhs.str;
	end_of_file = rhs.end_of_file;
	id = rhs.id;
	return *this;
}

shared_ptr<token_t> token_t::null()
{
	return shared_ptr<token_t>();
}

shared_ptr<token_t> token_t::from_string(fixed_string const& str)
{
	return shared_ptr<token_t>( new token_t(str.begin(), str.end()) );
}

code_span::code_span()
{
	set(0, 0, 0);
}

code_span::code_span( size_t line_beg, size_t col_beg, size_t line_end, size_t col_end )
	:line_beg(line_beg), line_end(line_end), col_beg(col_beg), col_end(col_end)
{

}

code_span::code_span( size_t line_beg, size_t col_beg, size_t length )
	: line_beg(line_beg), line_end(line_beg), col_beg(col_beg), col_end(length)
{
}

code_span code_span::merge( code_span const& s0, code_span const& s1 )
{
	code_span const *beg, *end;
	if( s0.line_beg < s1.line_beg ) {
		beg = &s0;
	} else if (s0.line_beg > s1.line_beg){
		beg = &s1;
	} else {
		beg = s0.col_beg < s1.col_beg ? &s0 : &s1;
	}

	if( s0.line_end < s1.line_end ) {
		end = &s1;
	} else if (s0.line_end > s1.line_end){
		end = &s0;
	} else {
		end = s0.col_end < s1.col_end ? &s1 : &s0;
	}

	return code_span(beg->line_beg, beg->col_beg, end->line_end, end->col_end);
}

void code_span::set( size_t line_beg, size_t col_beg, size_t line_end, size_t col_end )
{
	this->line_beg = line_beg;
	this->col_beg  = col_beg;
	this->line_end = line_end;
	this->col_end  = col_end;
}

void code_span::set( size_t line_beg, size_t col_beg, size_t length )
{
	this->line_beg = line_beg;
	this->col_beg  = col_beg;
	this->line_end = line_beg;
	this->col_end  = col_beg + length;
}

END_NS_SASL_COMMON();