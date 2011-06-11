#ifndef SASL_VM_OP_CODE_UTILITY_H
#define SASL_VM_OP_CODE_UTILITY_H

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/tuple/rem.hpp>

#define SASL_CALL_EXPANDED_INSTRUCTION( MACRO, INSTRUCTION_INFO )	\
	MACRO INSTRUCTION_INFO

#endif