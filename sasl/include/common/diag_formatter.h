#ifndef SASL_COMMON_DIAG_FORMATTER_H
#define SASL_COMMON_DIAG_FORMATTER_H

#include <sasl/include/common/common_fwd.h>

#include <string>

BEGIN_NS_SASL_COMMON();

class diag_item;

enum compiler_compatibility
{
	cc_msvc,
	cc_gcc
};

std::string str( diag_item const*, compiler_compatibility cc = cc_msvc );

END_NS_SASL_COMMON();

#endif