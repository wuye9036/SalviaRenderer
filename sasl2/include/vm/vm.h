#ifndef SASL_VM_VM_H
#define SASL_VM_VM_H

#include "forward.h"
#include "op_code.h"
#include "vm_stack.h"
#include <eflib/include/platform.h>
#include <vector>
#include <iostream>

BEGIN_NS_SASL_VM()

enum reg{
	r0 = 0,
	r1, r2, r3, r4,
	r5, r6, r7, r8, r9,
	r10, r11, r12, r13,
	r14, r15
};

struct instruction{
	instruction( op_code op ): op(op), arg0(0), arg1(0){}
	instruction( op_code op, intptr_t arg0 ): op(op), arg0(arg0), arg1(0){}
	instruction( op_code op, intptr_t arg0, intptr_t arg1 ): op(op), arg0(arg0), arg1(arg1){}

	op_code op;
	intptr_t arg0;
	intptr_t arg1;
};

class vm
{
public:
	typedef intptr_t	raw_t;

	typedef raw_t		address_t;
	typedef raw_t		regid_t;
	typedef raw_t		offset_t;
	typedef raw_t		intreg_t;

	static const raw_t i_register_count = 16;
	static const raw_t f_register_count = 16;

	vm(void);
	~vm(void);

	/********************************************************
	// 直接调用一组指令。
	// 将清空指令槽，eip置为0后开始调用，至op_halt退出。
	// 返回r[0]的值。
	********************************************************/
	intptr_t raw_call(const std::vector<instruction>& ins){
		op_buffer = ins;
		eip = 0;
		ebp = 0;
		while( step() ){
			;
		}
		return r[0];
	}

	bool step(){
		/*cout << "Instruction " << eip << " Executed." 
			<< " EBP: " << ebp 
			<< " ESP: " << esp() 
			<< " EIP: " << eip 
			<< endl;*/
		if ( eip >= (intptr_t)op_buffer.size() ){
			return false;
		}

		if ( execute_op( op_buffer[eip] ) ){
			jump();
			return true;
		}

		return false;
	}

	bool execute_op( const instruction& ins ){
		return execute_op( ins.op, ins.arg0, ins.arg1 );
	}

	bool execute_op(op_code op, intptr_t arg0, intptr_t arg1);

	//instruction operators
	void jump(){
		if ( jump_to == 0 ){
			++eip;
		} else {
			eip = jump_to;
		}

		jump_to = 0;
	}

	//virtual machine storage
	vm_stack<address_t> stack;
	std::vector<instruction> op_buffer;

	intptr_t eip;
	intptr_t ebp;

	intptr_t r[i_register_count];
	intptr_t f[f_register_count];

	intptr_t jump_to;
};

END_NS_SASL_VM()

#endif //SASL_VM_VM_H