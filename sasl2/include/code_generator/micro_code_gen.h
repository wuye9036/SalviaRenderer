#ifndef SASL_CODE_GENERATOR_MICRO_CODE_GEN_H
#define SASL_CODE_GENERATOR_MICRO_CODE_GEN_H

#include "forward.h"
#include "../vm/vm.h"
#include "../vm/op_code/typeless_opcode_generator.h"

BEGIN_NS_SASL_CODE_GENERATOR()

using sasl::vm::vm;
using sasl::vm::instruction;

class micro_code_gen{
	typedef vm::machine_t machine_t;

	std::vector<instruction> codes_;

	micro_code_gen& emit_op( vm::opcode_t op, vm::raw_t arg0 = 0, vm::raw_t arg1 = 0 );
public:
	const std::vector<instruction>& codes();

	/////////////////
	//	弱类型指令生成
	/////////////////
	SASL_DECL_TYPELESS_CODE_GENERATORS( micro_code_gen, SASL_VM_INSTRUCTIONS, machine_t );

	micro_code_gen& _add( vm::regid_t dest, vm::regid_t src );
	micro_code_gen& _load( vm::regid_t dest, vm::raw_t c );
};

END_NS_SASL_CODE_GENERATOR()
#endif