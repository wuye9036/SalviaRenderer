#pragma once

#include <vector>
#include <iostream>

using namespace std;

typedef unsigned char byte;

enum op_code{
	op_nop = 0,
	op_add,
	op_push,
	op_pop,
	op_loadrc,
	op_loadrs,	//从栈上读到内存中。第一个参数为寄存器号，第二个参数为栈基偏移
	op_halt,
	op_call,
	op_ret
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
	instruction( op_code op, int arg0 ): op(op), arg0(arg0), arg1(0){}
	instruction( op_code op, int arg0, int arg1 ): op(op), arg0(arg0), arg1(arg1){}

	op_code op;
	int arg0;
	int arg1;
};

class vm
{
public:
	vm(void);
	~vm(void);

	/********************************************************
	// 直接调用一组指令。
	// 将清空指令槽，eip置为0后开始调用，至op_halt退出。
	// 返回r[0]的值。
	********************************************************/
	int raw_call(const vector<instruction>& ins){
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
		if ( eip >= (int)op_buffer.size() ){
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

	bool execute_op(op_code op, int arg0, int arg1){
		switch (op)
		{
		case op_halt:
			return false;
		case op_nop:
			break;
		case op_add:
			r[arg0] += r[arg1];
			break;
		case op_push: 
			{
				int& pushed_reg( r[arg0] );
				stack_push( pushed_reg );
				break;
			}
		case op_pop:
			{
				size_t pop_size = r[arg0];
				stack_pop( pop_size );
				break;
			}
		case op_loadrc:
			{
				int& reg( r[arg0] );
				int val = arg1;
				reg = val;
				break;
			}
		case op_loadrs:
			{
				int& reg( r[arg0] );
				ebp_based_value( reg, arg1 );
				break;
			}
		case op_call:
			{
				int tar_addr = arg0;
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
		esp_based_value( v, - sizeof(ValueT) );
	}

	void stack_pop(size_t size){
		esp( esp() - (int)size );
	}

	template<typename ValueT>
	void stack_push( const ValueT& v ){
		stack.insert( stack.end(), (byte*)&v, (byte*)&v + sizeof(v) );	
	}

	int esp(){
		return (int) stack.size();
	}

	void esp( int addr ){
		stack.resize(addr);
	}

	//virtual machine storage
	vector<byte> stack;
	vector<instruction> op_buffer;

	int eip;
	int ebp;

	int r[16];
	int jump_to;
};
