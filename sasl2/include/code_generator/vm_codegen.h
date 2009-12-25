#ifndef SASL_CODE_GENERATOR_VM_CODEGEN_H
#define SASL_CODE_GENERATOR_VM_CODEGEN_H

#include "../vm/vm.h"
#include "../syntax_tree/expression.h"
#include "../syntax_tree/constant.h"
#include <bitset>
#include <cassert>

class vm_codegen{
	std::vector<instruction> ins_;
	std::bitset<vm::register_count> reg_usage;

public:
	vm_codegen(): reg_usage(0){
	}

	vm_codegen& emit_expression( const binary_expression& expr ){
		if ( expr.op != operators::add ){
			return *this;
		}

		int c0 = expr.left_expr->val;
		int c1 = expr.right_expr->val;

		vm::regid_t reg0 = allocate_reg();
		vm::regid_t reg1 = allocate_reg();

		_load_r_si( reg0, c0 );
		_load_r_si( reg1, c1 );
		_add_si( reg0, reg1 );
		
		free_reg(reg0);
		free_reg(reg1);

		return *this;
	}

	vm_codegen& emit_op( op_code op, vm::intreg_t arg0 = 0, vm::intreg_t arg1 = 0 ){
		ins_.push_back( instruction(op, arg0, arg1) );
		return *this;
	}
	const std::vector<instruction>& codes(){
		return ins_;
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

	//指令生成
	vm_codegen& _add_si( vm::regid_t dest, vm::regid_t src ){
		emit_op( op_add_si, dest, src );
		return *this;
	}

	vm_codegen& _load_r_si( vm::regid_t dest, vm::intreg_t c ){
		emit_op( op_load_r_si, dest, c );
		return *this;
	}

	// 将寄存器压到栈上，并释放寄存器占用
	vm_codegen& _save_r( vm::regid_t reg_id ){
		free_reg( reg_id );
		emit_op( op_push, reg_id );
		return *this;
	}

	// 将寄存器从栈上恢复，并设置寄存器被占用
	vm_codegen& _restore_r( vm::regid_t reg_id ){
		reallocate_reg( reg_id );
		emit_op( op_pop, reg_id );
		return *this;
	}
};

#endif //SASL_CODE_GENERATOR_VM_CODEGEN_H