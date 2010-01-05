#ifndef SASL_VM_OP_CODE_DISPATCHER_H
#define SASL_VM_OP_CODE_DISPATCHER_H

#include "parameter.h"
#include "processor.h"

#define SASL_DISPATCH_INSTRUCTION( INSTRUCTION, ARG0, ARG1 )					\
	case NS_SASL_VM_OP_CODE( SASL_IFN( INSTRUCTION ) ):							\
	{																			\
		SASL_PROCESSOR_NAME( INSTRUCTION ) (									\
			SASL_AGUMENT_VALUE_TO_PARAMETER( INSTRUCTION, 0, machine_t, ARG0),	\
			SASL_AGUMENT_VALUE_TO_PARAMETER( INSTRUCTION, 1, machine_t, ARG1)	\
			);																	\
		break;																	\
	}

#define SASL_DISPATCH_INSTRUCTIONS( INSTRUCTIONS, ARG0, ARG1 )	\
	BOOST_PP_SEQ_FOR_EACH( SASL_DISPATCH_INSTRUCTIONS_I, (ARG0, ARG1), INSTRUCTIONS )

#define SASL_DISPATCH_INSTRUCTIONS_I( r, data, elem )	\
	 SASL_DISPATCH_INSTRUCTION( elem, BOOST_PP_TUPLE_ELEM(2,0,data), BOOST_PP_TUPLE_ELEM(2,1,data) )

#endif