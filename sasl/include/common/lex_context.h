#ifndef SASL_COMMON_LEX_CONTEXT_H
#define SASL_COMMON_LEX_CONTEXT_H

#include "common_fwd.h"
#include <string>

BEGIN_NS_SASL_COMMON()

class lex_context{
	virtual const std::string& file_name() const = 0;
	virtual size_t column() const = 0;
	virtual size_t line() const = 0;

	virtual void next( const std::string& lit ) = 0;
};

END_NS_SASL_COMMON()

#endif