#include "../../include/vm/vm.h"

vm::vm(void): eip(0), jump_to(0), ebp(0){
}

vm::~vm(void)
{
}

bool vm::execute_op(op_code op, intptr_t arg0, intptr_t arg1){
	switch (op)
	{
	case op_halt:
		return false;
	case op_nop:
		break;
	case op_add_si:
		r[arg0] += r[arg1];
		break;
	case op_push: 
		{
			stack.push( r[arg0] );
			break;
		}
	case op_pop:
		{
			stack.pop( r[arg0] );
			break;
		}
	case op_load_r_si:
		{
			r[arg0] = arg1;
			break;
		}
	case op_load_r_s:
		{
			r[arg0] = stack.value_frame_based<intreg_t>( arg1 );
			break;
		}
	case op_call:
		{
			intptr_t tar_addr = arg0;
			stack.push( eip );
			stack.enter_frame();
			jump_to = tar_addr;
			break;
		}
	case op_ret:
		{
			stack.leave_frame();
			stack.pop( jump_to );
			++jump_to;
			break;
		}
	}

	return true;
}