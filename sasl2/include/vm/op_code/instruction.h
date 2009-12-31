#ifndef SASL_VM_OP_CODE_INSTRUCTION_H
#define SASL_VM_OP_CODE_INSTRUCTION_H

#include <boost/preprocessor.hpp>

/************************************
	定义Instruction ID及类型化的Instruction ID
************************************/
#define SASL_INSTRUCTION_ID_T( INSTRUCTION, ID )	\
	const int SASL_IFN(INSTRUCTION) = ID;
#define SASL_INSTRUCTION_ID_I( r, data, i, elem )	\
	SASL_INSTRUCTION_ID_T( elem, i )
#define SASL_VM_INSTRUCTIONS_ID( INSTRUCTION_SEQ )		\
	BOOST_PP_SEQ_FOR_EACH_I( SASL_INSTRUCTION_ID_I, _, INSTRUCTION_SEQ )

#define SASL_TYPED_INSTRUCTION( INSTRUCTION ) \
	instruction< SASL_IFN(INSTRUCTION) >

/*************************************
	定义Instruction的参数类型
*************************************/
#define SASL_INSTRUCTION_PARAMETER_STORAGE( INSTRUCTION, I )	\
	BOOST_PP_TUPLE_ELEM( 5, I*2+1, INSTRUCTION )

#define SASL_INSTRUCTION_PARAMETER_TYPE( INSTRUCTION, I )		\
	BOOST_PP_TUPLE_ELEM( 5, I*2+2, INSTRUCTION)

#define SASL_IPS( INSTRUCTION, I )	SASL_INSTRUCTION_PARAMETER_STORAGE( INSTRUCTION, I )
#define SASL_IPT( INSTRUCTION, I )	SASL_INSTRUCTION_PARAMETER_TYPE( INSTRUCTION, I )

#define SASL_PARAMETER_TYPE( INSTRUCTION, I, ADDRESS_T )				\
	parameter<															\
		NS_SASL_VM_OP_CODE_STORAGE( SASL_IPS(INSTRUCTION, I) ),			\
		NS_SASL_VM_OP_CODE_OPERAND_TYPE( SASL_IPT(INSTRUCTION, I) ),	\
		ADDRESS_T														\
	>

#define SASL_INSTRUCTION_INFO_T( INSTRUCTION )								\
	template<typename AddressT >											\
	struct instruction_info													\
		< SASL_TYPED_INSTRUCTION(INSTRUCTION), AddressT >					\
	{																		\
		typedef SASL_TYPED_INSTRUCTION( INSTRUCTION ) type;					\
		typedef SASL_PARAMETER_TYPE( INSTRUCTION, 0, AddressT ) p0_type;	\
		typedef SASL_PARAMETER_TYPE( INSTRUCTION, 1, AddressT ) p1_type;	\
	};

#endif SASL_VM_OP_CODE_MACROS_H