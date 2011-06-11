#ifndef SASL_VM_OP_CODE_INSTRUCTION_H
#define SASL_VM_OP_CODE_INSTRUCTION_H

#include "forward.h"
#include "naming.h"
#include <boost/preprocessor.hpp>

/************************************
	定义Instruction ID(OpCode)及类型化的Instruction ID
************************************/
#define SASL_INSTRUCTION_OPCODE_T( INSTRUCTION, ID )	\
	const int SASL_IFN(INSTRUCTION) = ID;
#define SASL_INSTRUCTION_OPCODE_I( r, data, i, elem )	\
	SASL_INSTRUCTION_OPCODE_T( elem, i )
#define SASL_VM_INSTRUCTIONS_OPCODE( INSTRUCTION_SEQ )		\
	BOOST_PP_SEQ_FOR_EACH_I( SASL_INSTRUCTION_OPCODE_I, _, INSTRUCTION_SEQ )

#define SASL_INSTRUCTION_TYPECODE( INSTRUCTION ) \
	typecode< SASL_IFN(INSTRUCTION) >

#define SASL_INSTRUCTION_FULL_TYPECODE( INSTRUCTION ) \
	NS_SASL_VM_OP_CODE( typecode< NS_SASL_VM_OP_CODE( SASL_IFN(INSTRUCTION) ) > )
/*************************************
	定义Instruction的参数类型
*************************************/
#define SASL_INSTRUCTION_PARAMETER_STORAGE( INSTRUCTION, I )	\
	BOOST_PP_TUPLE_ELEM( 5, BOOST_PP_ADD( BOOST_PP_MUL( I, 2 ), 1 ), INSTRUCTION )

#define SASL_INSTRUCTION_PARAMETER_TYPE( INSTRUCTION, I )		\
	BOOST_PP_TUPLE_ELEM( 5, BOOST_PP_ADD( BOOST_PP_MUL( I, 2 ), 2 ), INSTRUCTION)

#define SASL_IPS( INSTRUCTION, I )	SASL_INSTRUCTION_PARAMETER_STORAGE( INSTRUCTION, I )
#define SASL_IPT( INSTRUCTION, I )	SASL_INSTRUCTION_PARAMETER_TYPE( INSTRUCTION, I )

#define SASL_PARAMETER_TYPE( INSTRUCTION, I, MACHINE_T )				\
	parameter<															\
		SASL_NSSN( SASL_IPS(INSTRUCTION, I) ),			\
		SASL_OTFT( SASL_IPT(INSTRUCTION, I), MACHINE_T ),	\
		MACHINE_T														\
	>
#define SASL_FULL_PARAMETER_TYPE( INSTRUCTION, I, MACHINE_T )	\
	NS_SASL_VM_OP_CODE( SASL_PARAMETER_TYPE( INSTRUCTION, I, MACHINE_T) )
#define SASL_INSTRUCTION_INFO_T( INSTRUCTION )								\
	template<typename MachineT >											\
	struct instruction_info													\
		< SASL_INSTRUCTION_TYPECODE(INSTRUCTION), MachineT >				\
	{																		\
		typedef SASL_INSTRUCTION_TYPECODE( INSTRUCTION ) typecode;			\
		typedef SASL_PARAMETER_TYPE( INSTRUCTION, 0, MachineT ) p0_type;	\
		typedef SASL_PARAMETER_TYPE( INSTRUCTION, 1, MachineT ) p1_type;	\
	};

#endif SASL_VM_OP_CODE_MACROS_H