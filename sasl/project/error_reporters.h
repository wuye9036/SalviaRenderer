#ifndef ERRORREPORTERS_H_INCLUDED
#define ERRORREPORTERS_H_INCLUDED

#include "error_reporter.h"

class stdout_error_reporter : public error_reporter
{
public:
	virtual void report_compiler_internal_error(const std::string& errContent);
	virtual void report_syntax_error(const std::string& errContent);

	virtual stdout_error_reporter* clone();
};

#endif // ERRORREPORTERS_H_INCLUDED
