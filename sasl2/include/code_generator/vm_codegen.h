#ifndef SASL_CODE_GENERATOR_VM_CODEGEN_H
#define SASL_CODE_GENERATOR_VM_CODEGEN_H

#include "vm_storage.h"
#include "../vm/vm.h"
#include "../syntax_tree/expression.h"
#include "../syntax_tree/constant.h"
#include <bitset>
#include <cassert>

class micro_code_gen{
	std::vector<instruction> ins_;
public:
	micro_code_gen& emit_op( op_code op, vm::intreg_t arg0 = 0, vm::intreg_t arg1 = 0 ){
		ins_.push_back( instruction(op, arg0, arg1) );
		return *this;
	}

	const std::vector<instruction>& codes(){
		return ins_;
	}

	//指令生成
	micro_code_gen& _add_si( vm::regid_t dest, vm::regid_t src ){
		emit_op( op_add_si, dest, src );
		return *this;
	}

	micro_code_gen& _load_r_si( vm::regid_t dest, vm::intreg_t c ){
		emit_op( op_load_r_si, dest, c );
		return *this;
	}
};

class vm_codegen{
	micro_code_gen ins_;
	std::bitset<vm::register_count> reg_usage;

public:
	typedef vm_storage<vm::addr_t> storage_t;

	vm_codegen(): reg_usage(0){
	}

	storage_t emit_constant( const constant& c );
	vm_codegen& emit_expression( const binary_expression& expr ){
		if ( expr.op != operators::add ){
			return *this;
		}

		storage_t c0 = emit_constant( *expr.left_expr );
		storage_t c1 = emit_constant( *expr.right_expr );

		ins_._add_si( c0.addr, c1.addr );
		
		free_storage( c0 );
		free_storage( c1 );

		return *this;
	}


	const std::vector<instruction>& codes(){
		return ins_.codes();
	}

	//辅助函数
	vm_codegen& reallocate_reg( vm::regid_t reg_id ){
		assert( !reg_usage.test(reg_id) );
		reg_usage.set( reg_id, true );
		return *this;
	}

	vm::regid_t allocate_reg(){
		for( vm::regid_t id = 0; id < vm::regid_t(vm::register_count); ++id ){
			if ( !reg_usage.test(id) ){
				reallocate_reg( id );
				return id;
			}
		}
		assert(false);
		return -1;
	}

	vm_codegen& free_reg(vm::regid_t reg_id){
		assert( reg_usage.test( reg_id ) );
		reg_usage.set( reg_id, false );
		return *this;
	}


	// VM CODE GENERATOR 的扩展指令集。将会生成多条OP CODE
	// 将寄存器压到栈上，并释放寄存器占用
	void _save_r( vm::regid_t reg_id ){
		free_reg( reg_id );
		ins_.emit_op( op_push, reg_id );
	}

	// 将寄存器从栈上恢复，并设置寄存器被占用
	void _restore_r( vm::regid_t reg_id ){
		reallocate_reg( reg_id );
		ins_.emit_op( op_pop, reg_id );
	}


	void free_storage(const vm_codegen::storage_t& storage ){
		if ( storage.mode == storage_mode::register_id ){
			free_reg( vm::regid_t( storage.addr ) );
		}
	}
};

#endif //SASL_CODE_GENERATOR_VM_CODEGEN_H