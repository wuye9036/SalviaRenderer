#include "../../include/code_generator/micro_code_gen.h"

using namespace std;

BEGIN_NS_SASL_CODE_GENERATOR()

const vector<instruction>& micro_code_gen::codes(){
	return codes_;
}

micro_code_gen& micro_code_gen::_add_si( vm::regid_t dest, vm::regid_t src ){
	return emit_op( op_add_si, dest, src );
}

micro_code_gen& micro_code_gen::_load_r( vm::regid_t dest, vm::intreg_t c ){
	return emit_op( op_load_r, dest, c );
}

micro_code_gen& micro_code_gen::_load_i32r_fr( vm::regid_t dest, vm::intreg_t src){
	return emit_op( op_load_i32r_fr, dest, src );
}

micro_code_gen& micro_code_gen::_load_fr_i32r( vm::regid_t dest, vm::intreg_t src){
	return emit_op( op_load_fr_i32r, dest, src );
}

micro_code_gen& micro_code_gen::_push( vm::regid_t reg ){
	return emit_op( op_push, reg );
}

micro_code_gen& micro_code_gen::_pop( vm::regid_t reg ){
	return emit_op( op_pop, reg );
}

micro_code_gen& micro_code_gen::emit_op( op_code op, vm::intreg_t arg0, vm::intreg_t arg1 ){
	codes_.push_back( instruction(op, arg0, arg1) );
	return *this;
}

END_NS_SASL_CODE_GENERATOR()