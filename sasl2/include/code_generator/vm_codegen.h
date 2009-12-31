#ifndef SASL_CODE_GENERATOR_VM_CODEGEN_H
#define SASL_CODE_GENERATOR_VM_CODEGEN_H

#include "forward.h"
#include "vm_storage.h"
#include "micro_code_gen.h"
#include "../syntax_tree/expression.h"
#include "../syntax_tree/constant.h"
#include <bitset>
#include <cassert>

BEGIN_NS_SASL_CODE_GENERATOR()

using sasl::vm::vm;
using sasl::vm::instruction;

class vm_codegen{
public:
	typedef vm_storage<vm::address_t> storage_t;
	typedef boost::shared_ptr< storage_t > storage_ptr;
	typedef storage_t::address_t address_t;

	vm_codegen();

	storage_ptr emit_constant( const constant::handle_t& c );
	vm_codegen& emit_expression( const binary_expression::handle_t& expr );

	const std::vector<instruction>& codes();

	// VM CODE GENERATOR 的 mid level 指令集。将会生成多条OP CODE
	// 将寄存器压到栈上，并释放寄存器占用
	void _save_r( vm::regid_t reg_id );

	// 将寄存器从栈上恢复，并设置寄存器被占用
	void _restore_r( vm::regid_t reg_id );

private:
	micro_code_gen mcgen_;
	std::bitset<vm::i_register_count> reg_usage;

	boost::shared_ptr<storage_t> create_storage( storage_mode mode, address_t addr );
	void free_storage( storage_t& s );

	//辅助函数
	vm::regid_t allocate_reg();
	vm_codegen& reallocate_reg( vm::regid_t reg_id );
	vm_codegen& free_reg(vm::regid_t reg_id);

	class storage_deleter{
	public:
		storage_deleter( vm_codegen& vm ): vm(vm) {}
		storage_deleter( const storage_deleter& d ): vm(d.vm) {}

		//deleter
		void operator ()( storage_t* p );
	private:
		vm_codegen& vm;
	};
};

END_NS_SASL_CODE_GENERATOR()

#endif //SASL_CODE_GENERATOR_VM_CODEGEN_H