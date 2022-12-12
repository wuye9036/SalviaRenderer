#include <sasl/common/token.h>

using std::shared_ptr;
using std::make_shared;
using std::string_view;

namespace sasl::common {

token_t::token_t(): s("UNINITIALIZED_VALUE"), end_of_file(false)
{
}

token_t::token_t(const token_t& rhs) : file_name(rhs.file_name), span(rhs.span), s(rhs.s), id(rhs.id), end_of_file(rhs.end_of_file)
{
}

std::shared_ptr<token_t> token_t::make(
	size_t id, std::string_view s,
	size_t line, size_t col, std::string_view fname
	)
{
	std::shared_ptr<token_t> ret = std::make_shared<token_t>();
	ret->id = id;
	ret->s = s;
	ret->file_name = fname;
	ret->span.set( line, col, 0 );
	ret->end_of_file = false;

	size_t cur_line = line;
	size_t cur_col = col;
	for(auto ch: s)
	{
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
	s = rhs.s;
	end_of_file = rhs.end_of_file;
	id = rhs.id;
	return *this;
}

shared_ptr<token_t> token_t::null()
{
	return shared_ptr<token_t>();
}

shared_ptr<token_t> token_t::from_string(string_view str)
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

}