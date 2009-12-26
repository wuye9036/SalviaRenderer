#include "../../include/code_generator/vm_codegen.h"

vm_codegen::storage_t vm_codegen::emit_constant( const constant& t ){
	vm::regid_t reg = allocate_reg();
	ins_._load_r_si( reg, t.val );
	return storage_t( storage_mode::register_id, reg );
}