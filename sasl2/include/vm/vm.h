#pragma once

#include <vector>
#include <iostream>

typedef unsigned char byte;

enum op_code{
	/*********************************
	*	辅助
	*********************************/
	op_nop = 0,
	// 描述：		空指令
	// 指令格式：	op_nop
	// 参数：		无
	op_halt,
	// 描述：		在任意状态下，终止虚拟机的执行并停机
	// 指令格式：	op_halt
	// 参数：		无

	/*********************************
	*	整数代数运算
	*********************************/
	op_add_si,
	// 描述：		将两个寄存器中的数以有符号整数的形式相加。
	// 指令格式：	op_add_si REG0, REG1
	// 参数：		REG0: 目标寄存器号，REG1: 源寄存器号

	/*********************************
	*	栈
	*********************************/
	op_push,
	// 描述：		将寄存器压入栈中。
	// 指令格式：	op_push REG0
	// 参数：		REG0：要压入栈的寄存器号。
	op_pop,
	// 描述：		将栈中数据返回寄存器，并从栈中弹出相应数据。
	// 指令格式：	op_pop REG0
	// 参数：		REG0：接受返回值的寄存器号。

	/*********************************
	*	拷贝
	*********************************/
	op_load_r_si,
	// 描述：		读入带符号常数到寄存器中
	// 指令格式：	op_load_r_si REG0, CONST
	// 参数：		REG0：目标寄存器号，CONST：有符号整数
	op_load_r_ui,
	// 描述：		读入无符号常数到寄存器中
	// 指令格式：	op_load_r_ui REG0, CONST
	// 参数：		REG0：目标寄存器号，CONST：无符号整数
	op_load_r_ssi,
	// 描述：		从栈中读取一个有符号数到寄存器中
	// 指令格式：	op_load_r_ssi REG0, OFFSET
	// 参数：		REG0：目标寄存器，CONST：栈基偏移地址

	/*********************************
	*	子过程调用
	*********************************/	
	op_call,
	// 描述：		调用指令。依次将ebp和eip压栈，并跳转至目标地址。
	// 指令格式：	op_call ADDR
	// 参数：		ADDR：指令地址
	op_ret
	// 描述：		调用返回指令。清理子过程栈的使用，恢复父过程的栈基指针，并跳转会父指令的位置。
	// 指令格式：	op_ret
	// 参数：		无
};

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
	typedef intptr_t intreg_t;
	typedef uintptr_t uintreg_t;
	typedef intptr_t regid_t;
	typedef intptr_t addr_t;
	typedef intptr_t offset_t;

	static const int register_count = 16;

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

	bool execute_op(op_code op, intptr_t arg0, intptr_t arg1){
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
				intptr_t& pushed_reg( r[arg0] );
				stack_push( pushed_reg );
				break;
			}
		case op_pop:
			{
				stack_back_value( r[arg0] );
				stack_pop( sizeof(r[arg0]) );
				break;
			}
		case op_load_r_si:
			{
				intptr_t& reg( r[arg0] );
				intptr_t val = arg1;
				reg = val;
				break;
			}
		case op_load_r_ssi:
			{
				intptr_t& reg( r[arg0] );
				ebp_based_value( reg, arg1 );
				break;
			}
		case op_call:
			{
				intptr_t tar_addr = arg0;
				stack_push( esp() );
				stack_push( eip );
				ebp = esp();
				jump_to = tar_addr;
				break;
			}
		case op_ret:
			{
				esp(ebp);
				stack_back_value( jump_to );
				stack_pop( sizeof( jump_to ) );
				stack_back_value( ebp );
				stack_pop( sizeof ( ebp ) );
				++jump_to;
				break;
			}
		}

		return true;
	}

	//instruction operators
	void jump(){
		if ( jump_to == 0 ){
			++eip;
		} else {
			eip = jump_to;
		}

		jump_to = 0;
	}

	//stack operators
	void* stack_pos( intptr_t offset ){
		return (void*)( (byte*)&(stack[0]) + offset );
	}

	template <typename ValueT>
	void stack_value(ValueT& v, intptr_t addr){
		memcpy( &v, stack_pos( addr ), sizeof(ValueT) );
	}

	template <typename ValueT>
	void ebp_based_value(ValueT& v, intptr_t offset){
		stack_value( v, ebp + offset );
	}

	template <typename ValueT>
	void esp_based_value(ValueT& v, intptr_t offset){
		stack_value( v, esp() + offset );
	}

	template <typename ValueT>
	void stack_back_value(ValueT& v){
		esp_based_value( v, - ((intptr_t)sizeof(ValueT)) );
	}

	void stack_pop(size_t size){
		esp( esp() - (intptr_t)size );
	}

	template<typename ValueT>
	void stack_push( const ValueT& v ){
		stack.insert( stack.end(), (byte*)&v, (byte*)&v + sizeof(v) );	
	}

	intptr_t esp(){
		return (intptr_t) stack.size();
	}

	void esp( intptr_t addr ){
		stack.resize(addr);
	}

	//virtual machine storage
	std::vector<byte> stack;
	std::vector<instruction> op_buffer;

	intptr_t eip;
	intptr_t ebp;

	intptr_t r[register_count];
	intptr_t jump_to;
};
