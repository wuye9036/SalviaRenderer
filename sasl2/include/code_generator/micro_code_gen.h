#ifndef SASL_CODE_GENERATOR_MICRO_CODE_GEN_H
#define SASL_CODE_GENERATOR_MICRO_CODE_GEN_H

#include "../vm/vm.h"

class micro_code_gen{
	std::vector<instruction> codes_;

	micro_code_gen& emit_op( op_code op, vm::intreg_t arg0 = 0, vm::intreg_t arg1 = 0 );
public:
	const std::vector<instruction>& codes();

	/////////////////
	//	÷∏¡Ó…˙≥…
	/////////////////
	micro_code_gen& _push( vm::regid_t reg );
	micro_code_gen& _pop( vm::regid_t reg );

	micro_code_gen& _add_si( vm::regid_t dest, vm::regid_t src );

	micro_code_gen& _load_r_si( vm::regid_t dest, vm::intreg_t c );
};

#endif