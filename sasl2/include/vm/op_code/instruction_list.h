#ifndef SASL_VM_OP_CODE_INSTRUCTION_LIST_H
#define SASL_VM_OP_CODE_INSTRUCTION_LIST_H

#include "forward.h"
#include "macros.h"
#include <boost/config.hpp>
/**************************
	指令集定义处。
	注意：如果有新的指令，请在此处添加声明。
**************************/

#define SASL_VM_INSTRUCTIONS			\
	(add,	stk, i32, stk, i32 )		\
	(load,	gr,  raw, c,   i32 )		\
	\
	/***
	***/

BEGIN_NS_SASL_VM_OP_CODE()

SASL_VM_INSTRUCTIONS_ID( SASL_VM_INSTRUCTIONS );

template <int InstructioID> struct instruction{
	static const int id = InstructionID;
};

template <typename InstructionT, typename AddressT>
struct instruction_info{
	typedef InstructionT type;
	typedef parameter<storage::_, value_type::_, AddressT> p0_t;
	typedef parameter<storage::_, value_type::_, AddressT> p1_t;
};

END_NS_SASL_VM_OP_CODE()

#endif