#include <sasl/include/code_generator/llvm/cgllvm_contexts.h>

#include <eflib/include/diagnostics/assert.h>

BEGIN_NS_SASL_CODE_GENERATOR();


cgllvm_sctxt_data::cgllvm_sctxt_data(){
	memset( this, 0, sizeof(*this) );
}


cgllvm_sctxt::cgllvm_sctxt()
{
}

cgllvm_sctxt::cgllvm_sctxt( cgllvm_sctxt const& rhs ){
	copy(&rhs);
}

cgllvm_sctxt_data& cgllvm_sctxt::data(){
	return hold_data;
}

cgllvm_sctxt_data const& cgllvm_sctxt::data() const{
	return hold_data;
}

void cgllvm_sctxt::data( cgllvm_sctxt_data const& rhs ){
	if( &rhs == &data() ) return;
	hold_data = rhs;
}

void cgllvm_sctxt::data( cgllvm_sctxt const* rhs ){
	hold_data = rhs->data();
}

void cgllvm_sctxt::storage( cgllvm_sctxt const* rhs ){
	data().is_ref = rhs->data().is_ref;
	data().val = rhs->data().val;
	data().global = rhs->data().global;
	data().local = rhs->data().local;
	data().agg = rhs->data().agg;
	data().hint_name = rhs->data().hint_name;
}

void cgllvm_sctxt::type( cgllvm_sctxt const* rhs ){
	data().val_type = rhs->data().val_type;
	data().is_signed = rhs->data().is_signed;
}

void cgllvm_sctxt::storage_and_type( cgllvm_sctxt* rhs ){
	storage(rhs);
	type(rhs);
}

void cgllvm_sctxt::copy( cgllvm_sctxt const* rhs ){
	env( rhs->env() );
	data( rhs->data() );
}

cgllvm_sctxt_env& cgllvm_sctxt::env(){
	return hold_env;
}

cgllvm_sctxt_env const& cgllvm_sctxt::env() const{
	return hold_env;
}

void cgllvm_sctxt::env( cgllvm_sctxt const* rhs ){
	hold_env = rhs->env();
}

void cgllvm_sctxt::env( cgllvm_sctxt_env const& rhs ){
	hold_env = rhs;
}

cgllvm_sctxt& cgllvm_sctxt::operator=( cgllvm_sctxt const& rhs ){
	copy( &rhs );
	return *this;
}

void cgllvm_sctxt::clear_data(){
	data( cgllvm_sctxt().data() );
}

cgllvm_sctxt_env::cgllvm_sctxt_env() 
	: parent_fn(NULL), block(NULL), parent_struct(NULL),
	is_semantic_mode(false), declarator_type(NULL)
{
}

END_NS_SASL_CODE_GENERATOR();