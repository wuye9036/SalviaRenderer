#ifndef SASL_COMMON_LEX_CONTEXT_H
#define SASL_COMMON_LEX_CONTEXT_H

#include <sasl/include/common/common_fwd.h>
#include <eflib/include/string/ustring.h>

BEGIN_NS_SASL_COMMON();

class lex_context{
public:
	virtual eflib::fixed_string const&
					file_name() const = 0;
	virtual size_t	column() const = 0;
	virtual size_t	line() const = 0;
	virtual void	update_position(eflib::fixed_string const& lit) = 0;
};


class code_source{
public:
	virtual bool				failed()= 0;
	virtual bool				eof()	= 0;
	virtual eflib::fixed_string	next()	= 0;
	virtual eflib::fixed_string	error()	= 0;
};

END_NS_SASL_COMMON();

#endif