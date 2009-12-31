#ifndef SASL_CODE_GENERATOR_MICRO_CODE_GEN_H
#define SASL_CODE_GENERATOR_MICRO_CODE_GEN_H

#include "forward.h"
#include "../vm/vm.h"

BEGIN_NS_SASL_CODE_GENERATOR()

using sasl::vm::vm;
using sasl::vm::instruction;

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

	micro_code_gen& _load_r( vm::regid_t dest, vm::intreg_t c );
	micro_code_gen& _load_i32r_fr( vm::regid_t dest, vm::intreg_t src );
	micro_code_gen& _load_fr_i32r( vm::regid_t dest, vm::intreg_t src );
};

END_NS_SASL_CODE_GENERATOR()
#endif