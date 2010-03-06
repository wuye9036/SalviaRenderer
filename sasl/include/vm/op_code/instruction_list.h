#ifndef SASL_VM_OP_CODE_INSTRUCTION_LIST_H
#define SASL_VM_OP_CODE_INSTRUCTION_LIST_H

#include "forward.h"
#include "instruction.h"
#include "parameter.h"
#include "storage.h"
#include "operand_type.h"

/**************************
	指令集定义处。
	注意：如果有新的指令，请在此处添加声明。
**************************/

#define SASL_VM_INSTRUCTIONS		\
	((add,	gr, raw, gr, raw ))		\
	((sub,  gr, raw, gr, raw ))		\
	((mul,  gr, raw, gr, raw ))		\
	((div,  gr, raw, gr, raw ))		\
	((load,	gr, raw, c,  raw ))		\
	((push, gr, raw, _,  _))		\
	((pop,  gr, raw, _,  _))		\
	\
	/***
	***/

BEGIN_NS_SASL_VM_OP_CODE()

SASL_VM_INSTRUCTIONS_OPCODE( SASL_VM_INSTRUCTIONS );

template <int Opcode> struct typecode{
	static const int id = Opcode;
};

template <typename InstructionT, typename MachineT>
struct instruction_info{
	typedef InstructionT type;
	typedef parameter< SASL_NSSN(_), typename SASL_OTFT(_, MachineT), MachineT> p0_t;
	typedef parameter< SASL_NSSN(_), typename SASL_OTFT(_, MachineT), MachineT> p1_t;
};

END_NS_SASL_VM_OP_CODE()

#endif