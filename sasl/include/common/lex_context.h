#ifndef SASL_COMMON_LEX_CONTEXT_H
#define SASL_COMMON_LEX_CONTEXT_H

#include <sasl/include/common/common_fwd.h>
#include <string>

BEGIN_NS_SASL_COMMON();

class lex_context{
public:
	virtual std::string const&	file_name() const = 0;
	virtual size_t				column() const = 0;
	virtual size_t				line() const = 0;
	virtual void				update_position( std::string const& lit ) = 0;
};


class code_source{
public:
	virtual bool		failed()= 0;
	virtual bool		eof()	= 0;
	virtual std::string	next()	= 0;
	virtual std::string	error()	= 0;
};

END_NS_SASL_COMMON();

#endif