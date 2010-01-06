#include "../../include/vm/vm.h"

BEGIN_NS_SASL_VM()

vm::vm(void): eip(0), jump_to(0), ebp(0){
}

vm::~vm(void)
{
}

SASL_PROCESSOR_IMPL_CONVERT_PARAMETER_TO_VALUE_REFERENCE( vm, (add, gr, raw, gr, raw) )
SASL_PROCESSOR_IMPL_CONVERT_PARAMETER_TO_VALUE_REFERENCE( vm, (load, gr, raw, c, raw) )

bool vm::execute_op(opcode_t op, operand_t arg0, operand_t arg1){
	switch( op ){
		SASL_DISPATCH_INSTRUCTIONS( SASL_VM_INSTRUCTIONS, arg0, arg1 )
	}
	return true;
}


END_NS_SASL_VM()