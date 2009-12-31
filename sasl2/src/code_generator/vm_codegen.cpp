#include "../../include/code_generator/vm_codegen.h"

using namespace boost;

BEGIN_NS_SASL_CODE_GENERATOR()

vm_codegen::storage_ptr vm_codegen::emit_constant( const constant::handle_t& t ){
	vm::regid_t reg = allocate_reg();
	mcgen_._load_r( reg, t->val );
	return create_storage( storage_mode::register_id, reg );
}

shared_ptr<vm_codegen::storage_t> vm_codegen::create_storage( storage_mode mode, vm_codegen::address_t addr ){
	return shared_ptr<vm_codegen::storage_t>( new vm_codegen::storage_t(mode, addr), storage_deleter(*this) );	
}

void vm_codegen::_save_r( vm::regid_t reg_id )
{
	free_reg( reg_id );
	mcgen_._push( reg_id );
}

void vm_codegen::_restore_r( vm::regid_t reg_id )
{
	reallocate_reg( reg_id );
	mcgen_._pop( reg_id );
}

vm::regid_t vm_codegen::allocate_reg()
{
	for( vm::regid_t id = 0; id < vm::regid_t(vm::i_register_count); ++id ){
		if ( !reg_usage.test(id) ){
			reallocate_reg( id );
			return id;
		}
	}
	assert(false);
	return -1;
}

vm_codegen& vm_codegen::free_reg( vm::regid_t reg_id )
{
	assert( reg_usage.test( reg_id ) );
	reg_usage.set( reg_id, false );
	return *this;
}

void vm_codegen::free_storage( storage_t& s )
{
	if ( s.mode == storage_mode::register_id ){
		free_reg( s.addr );
	}
}

vm_codegen& vm_codegen::emit_expression( const binary_expression::handle_t& expr )
{
	if ( expr->op->op != operators::add ){
		return *this;
	}

	{
		storage_ptr c0 = emit_constant( expr->left_expr );
		storage_ptr c1 = emit_constant( expr->right_expr );

		mcgen_._add_si( c0->addr, c1->addr );
	}


	return *this;
}

const std::vector<instruction>& vm_codegen::codes()
{
	return mcgen_.codes();
}

vm_codegen& vm_codegen::reallocate_reg( vm::regid_t reg_id )
{
	assert( !reg_usage.test(reg_id) );
	reg_usage.set( reg_id, true );
	return *this;
}

vm_codegen::vm_codegen() : reg_usage(0){
}

void vm_codegen::storage_deleter::operator()( storage_t* p )
{
	if(p == NULL) return;
	vm.free_storage(*p);
	delete p;
}

END_NS_SASL_CODE_GENERATOR()