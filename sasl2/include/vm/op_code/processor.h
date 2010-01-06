#ifndef SASL_VM_OP_CODE_PROCESSOR_H
#define SASL_VM_OP_CODE_PROCESSOR_H

#include "forward.h"
#include "naming.h"
#include "parameter.h"
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/seq.hpp>
#include <boost/utility/enable_if.hpp>

///////////////////////////
// Processor Naming
///////////////////////////
#define SASL_PROCESSOR_NAME_ISN( INSTRUCTION_SHORT_NAME ) BOOST_PP_CAT(INSTRUCTION_SHORT_NAME, _processor)
#define SASL_PROCESSOR_NAME( INSTRUCTION ) SASL_PROCESSOR_NAME_ISN( SASL_ISN( INSTRUCTION ) )

///////////////////////////
// Processor Declaration
///////////////////////////
#define SASL_PROCESSOR_DECL( INSTRUCTION, P0_TYPE, P1_TYPE )	\
	void SASL_PROCESSOR_NAME( INSTRUCTION ) (			\
		P0_TYPE arg0,									\
		P1_TYPE arg1									\
		)

#define SASL_SPECIALIZED_PROCESSOR_DECL_ISN( INSTRUCTION_SHORT_NAME, P0_TYPE, P1_TYPE, ENABLE_IF_TYPE )\
	void SASL_PROCESSOR_NAME_ISN(INSTRUCTION_SHORT_NAME) (	\
		P0_TYPE arg0,										\
		P1_TYPE arg1,										\
		ENABLE_IF_TYPE dummy = 0							\
		)

#define SASL_PROCESSOR_DECL_DEFAULT( INSTRUCTION, MACHINE_T ) 		\
	SASL_PROCESSOR_DECL( INSTRUCTION,								\
		SASL_FULL_PARAMETER_TYPE(INSTRUCTION, 0, MACHINE_T)&,	\
		SASL_FULL_PARAMETER_TYPE(INSTRUCTION, 1, MACHINE_T)&	\
		)

///////////////////////////
// Processor Implementation Signature
///////////////////////////
#define SASL_PROCESSOR_IMPL( CLASS_NAME, INSTRUCTION, P0_TYPE, P1_TYPE)	\
	SASL_PROCESSOR_IMPL_ISN( CLASS_NAME, SASL_ISN(INSTRUCTION), P0_TYPE, P1_TYPE )

#define SASL_PROCESSOR_IMPL_ISN( CLASS_NAME, INSTRUCTION_SHORT_NAME, P0_TYPE, P1_TYPE )	\
	void BOOST_PP_CAT(CLASS_NAME,::)SASL_PROCESSOR_NAME_ISN( INSTRUCTION_SHORT_NAME )(		\
		P0_TYPE arg0,																		\
		P1_TYPE arg1																		\
		)

#define SASL_PROCESSOR_IMPL_DEFAULT( CLASS_NAME, INSTRUCTION )	\
	SASL_PROCESSOR_IMPL( CLASS_NAME, INSTRUCTION,					\
		SASL_FULL_PARAMETER_TYPE(INSTRUCTION, 0, BOOST_PP_CAT(CLASS_NAME, ::machine_t) )&,	\
		SASL_FULL_PARAMETER_TYPE(INSTRUCTION, 1, BOOST_PP_CAT(CLASS_NAME, ::machine_t) )&	\
		)

//////////////////////////////////////////
// Predefined Processor Implementation
//////////////////////////////////////////
#define SASL_PROCESSOR_IMPL_TYPE_CONVERTING( CLASS_NAME, INSTRUCTION, P0_CONVERTED_TYPE, P1_CONVERTED_TYPE )	\
	SASL_PROCESSOR_IMPL_DEFAULT( CLASS_NAME, INSTRUCTION ){	\
		SASL_PROCESSOR_NAME( INSTRUCTION )(				\
			convert_parameter(arg0, P0_CONVERTED_TYPE() ),	\
			convert_parameter(arg1, P1_CONVERTED_TYPE() )	\
		);												\
	}

#define SASL_PROCESSOR_IMPL_CONVERT_PARAMETER_TO_VALUE_REFERENCE( CLASS_NAME, INSTRUCTION )	\
	SASL_PROCESSOR_IMPL_TYPE_CONVERTING(	\
		CLASS_NAME, INSTRUCTION,			\
		SASL_FULL_PARAMETER_TYPE(INSTRUCTION, 0, BOOST_PP_CAT(CLASS_NAME, ::machine_t))::value_t_tag,	\
		SASL_FULL_PARAMETER_TYPE(INSTRUCTION, 1, BOOST_PP_CAT(CLASS_NAME, ::machine_t))::value_t_tag	\
		)

///////////////////////////////////////
// Processor Calling
///////////////////////////////////////
#define SASL_CALL_PROCESSOR( INSTRUCTION, ARG0, ARG1 )	\
	SASL_PROCESSOR_NAME( INSTRUCTION ) ( ARG0, ARG1 )

////////////////////////////////////////
//	batched instruction processor declaration support
////////////////////////////////////////
#define SASL_DECLARE_DEFAULT_PROCESSOR_I( r, data, elem )	\
	SASL_PROCESSOR_DECL_DEFAULT( elem, data ) ;
#define SASL_DECLARE_DEFAULT_PROCESSORS( INSTRUCTION_SEQ, MACHINE_T )	\
	BOOST_PP_SEQ_FOR_EACH( SASL_DECLARE_DEFAULT_PROCESSOR_I, MACHINE_T, INSTRUCTION_SEQ )

///////////////////////////////
// Compile Conditions
///////////////////////////////
#define SASL_ENABLE_IF_STORAGE(PARAMETER_T, STORAGE) typename boost::enable_if< boost::is_same< typename BOOST_PP_CAT(PARAMETER_T, ::storage_tag), SASL_NAMESPACED_STORAGE_NAME( STORAGE ) > >::type* dummy = 0
#define SASL_DISABLE_IF_PARAMETER(T) typename boost::disable_if< NS_SASL_VM_OP_CODE(is_parameter< T >) >::type*

#endif