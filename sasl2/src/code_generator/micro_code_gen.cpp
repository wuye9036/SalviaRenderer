#include "../../include/code_generator/micro_code_gen.h"
#include "../../include/vm/op_code/instruction_list.h"
using namespace std;

BEGIN_NS_SASL_CODE_GENERATOR()

const vector<instruction>& micro_code_gen::codes(){
	return codes_;
}

micro_code_gen& micro_code_gen::_add( vm::regid_t dest, vm::regid_t src ){
	return emit_op( NS_SASL_VM_OP_CODE(add_gr_raw_gr_raw), dest, src );
}

micro_code_gen& micro_code_gen::_load( vm::regid_t dest, vm::raw_t c ){
	return emit_op( NS_SASL_VM_OP_CODE(load_gr_raw_c_raw), dest, c );
}

micro_code_gen& micro_code_gen::emit_op( vm::opcode_t op, vm::raw_t arg0, vm::raw_t arg1 ){
	codes_.push_back( instruction(op, arg0, arg1) );
	return *this;
}

END_NS_SASL_CODE_GENERATOR()