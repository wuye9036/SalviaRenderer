#include <sasl/include/code_generator/llvm/cgllvm_contexts.h>
#include <sasl/include/code_generator/llvm/cgllvm_service.h>

#include <eflib/include/diagnostics/assert.h>

BEGIN_NS_SASL_CODE_GENERATOR();

cgllvm_sctxt_data::cgllvm_sctxt_data()
: declarator_count(0), semantic_mode(false)
{
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

void cgllvm_sctxt::copy( cgllvm_sctxt const* rhs ){
	if( rhs == this ){ return; }
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

value_t const& cgllvm_sctxt::value() const{
	return data().val;
}

value_t& cgllvm_sctxt::value(){
	return data().val;
}

value_t cgllvm_sctxt::get_rvalue() const{
	return data().val.to_rvalue();
}

value_tyinfo* cgllvm_sctxt::get_typtr() const
{
	return get_tysp().get();
}

boost::shared_ptr<value_tyinfo> cgllvm_sctxt::get_tysp() const{
	return data().tyinfo;
}

cgllvm_sctxt_env::cgllvm_sctxt_env() 
	: block(NULL)
	, parent_struct(NULL)
	, is_semantic_mode(false)
{
}

END_NS_SASL_CODE_GENERATOR();