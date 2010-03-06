#ifndef SASL_VM_VM_H
#define SASL_VM_VM_H

#include "forward.h"
#include "op_code.h"
#include "op_code/instruction_list.h"
#include "op_code/processor.h"
#include "op_code/dispatcher.h"

#include "vm_stack.h"
#include <eflib/include/platform.h>
#include <boost/utility/addressof.hpp>
#include <vector>
#include <iostream>

BEGIN_NS_SASL_VM()

enum reg{
	r0 = 0,
	r1, r2, r3, r4,
	r5, r6, r7, r8, r9,
	r10, r11, r12, r13,
	r14, 
};

template< typename RawT >
struct machine_information{
	typedef RawT raw_t;

	typedef raw_t address_t;
	typedef raw_t regid_t;
	typedef raw_t offset_t;
	typedef raw_t operand_t;

	typedef int opcode_t;
};

typedef machine_information<intptr_t> machine_t;

struct instruction: public machine_t{
	instruction( opcode_t op ): op(op), arg0(0), arg1(0){}
	instruction( opcode_t op, operand_t arg0 ): op(op), arg0(arg0), arg1(0){}
	instruction( opcode_t op, operand_t arg0, operand_t arg1 ): op(op), arg0(arg0), arg1(arg1){}

	opcode_t op;
	operand_t arg0;
	operand_t arg1;
};

class vm : public machine_t
{
public:
	typedef machine_t machine_t;

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

	bool execute_op(opcode_t op, operand_t arg0, operand_t arg1);

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

private:
	// 参数转换函数
	template <typename ParameterT>
	typename ParameterT::value_t* convert_parameter( const ParameterT& par, const typename ParameterT::value_t_tag*, SASL_ENABLE_IF_STORAGE(ParameterT, _) ){
		// 稻草人函数，只是返回一个有效地址而已。
		return reinterpret_cast<typename ParameterT::value_t*>(const_cast<address_t*>(boost::addressof(par.addr)));
	}

	template <typename ParameterT>
	typename ParameterT::value_t* convert_parameter( const ParameterT& par, const typename ParameterT::value_t_tag*, SASL_ENABLE_IF_STORAGE(ParameterT, c) ){
		return reinterpret_cast<typename ParameterT::value_t*>(const_cast<address_t*>(boost::addressof(par.addr)));
	}

	template <typename ParameterT>
	typename ParameterT::value_t* convert_parameter( const ParameterT& par, const typename ParameterT::value_t_tag*, SASL_ENABLE_IF_STORAGE(ParameterT, gr) ){
		ParameterT::value_t* retptr = boost::addressof(r[par.addr]);
		return retptr;
	}

	template <typename ParameterT>
	typename ParameterT::value_t* convert_parameter( const ParameterT& par, const typename ParameterT::value_t_tag*, SASL_ENABLE_IF_STORAGE(ParameterT, a) ){
		ParameterT::value_t* retptr = reinterpret_cast<typename ParameterT::value_t*>( par.addr );
		return retptr;
	}

	template <typename ParameterT>
	typename ParameterT::value_t* convert_parameter( const ParameterT& par, const typename ParameterT::value_t_tag*, SASL_ENABLE_IF_STORAGE(ParameterT, ia) ){
		address_t value_addr = *(reinterpret_cast<address_t*>( par.addr ));
		typename ParameterT::value_t* retptr = reinterpret_cast<typename ParameterT::value_t*>( value_addr );
		return retptr;
	}

	template <typename ParameterT>
	typename ParameterT::value_t* convert_parameter( const ParameterT& par, const typename ParameterT::value_t_tag*, SASL_ENABLE_IF_STORAGE(ParameterT, igr) ){
		address_t value_addr = r[par.addr];
		typename ParameterT::value_t* retptr = reinterpret_cast<typename ParameterT::value_t*>( value_addr );
		return retptr;
	}

	template <typename ParameterT>
	typename ParameterT::value_t& convert_parameter( const ParameterT& par, const typename ParameterT::value_t_tag& ){
		return *( convert_parameter( par, (const typename ParameterT::value_t_tag*)(NULL) ) );
	}

	// 预定义的声明
	SASL_DECLARE_DEFAULT_PROCESSORS( SASL_VM_INSTRUCTIONS, machine_t );
	
	// 实际的处理函数
	template <typename ValueT>
	SASL_SPECIALIZED_PROCESSOR_DECL_ISN( push, ValueT&, SASL_OPERAND_TYPE(_, machine_t)&, SASL_DISABLE_IF_PARAMETER(ValueT) ){
		stack.push(arg0);
	}

	template <typename ValueT>
	SASL_SPECIALIZED_PROCESSOR_DECL_ISN( pop, ValueT&, SASL_OPERAND_TYPE(_, machine_t)&, SASL_DISABLE_IF_PARAMETER(ValueT) ){
		arg0 = stack.pop<ValueT>();
	}

	template <typename ValueT>
	SASL_SPECIALIZED_PROCESSOR_DECL_ISN( add, ValueT&, ValueT&, SASL_DISABLE_IF_PARAMETER(ValueT) ){
		arg0 += arg1;
	}

	template <typename ValueT>
	SASL_SPECIALIZED_PROCESSOR_DECL_ISN( sub, ValueT&, ValueT&, SASL_DISABLE_IF_PARAMETER(ValueT) ){
		arg0 -= arg1;
	}

	template <typename ValueT>
	SASL_SPECIALIZED_PROCESSOR_DECL_ISN( mul, ValueT&, ValueT&, SASL_DISABLE_IF_PARAMETER(ValueT) ){
		arg0 *= arg1;
	}

	template <typename ValueT>
	SASL_SPECIALIZED_PROCESSOR_DECL_ISN( div, ValueT&, ValueT&, SASL_DISABLE_IF_PARAMETER(ValueT) ){
		arg0 /= arg1;
	}

	template <typename ValueT>
	SASL_SPECIALIZED_PROCESSOR_DECL_ISN( load, ValueT&, ValueT&, SASL_DISABLE_IF_PARAMETER(ValueT) ){
		arg0 = arg1;
	}
};

END_NS_SASL_VM()

#endif //SASL_VM_VM_H