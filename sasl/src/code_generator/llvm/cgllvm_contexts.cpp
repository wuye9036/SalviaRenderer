#include <sasl/include/code_generator/llvm/cgllvm_contexts.h>

#include <eflib/include/diagnostics/assert.h>

BEGIN_NS_SASL_CODE_GENERATOR();


cgllvm_sctxt_data::cgllvm_sctxt_data(){
	memset( this, 0, sizeof(*this) );
}


cgllvm_sctxt::cgllvm_sctxt()
{
}

cgllvm_sctxt_data& cgllvm_sctxt::data(){
	return hold_data;
}

cgllvm_sctxt_data const& cgllvm_sctxt::data() const{
	return hold_data;
}

void cgllvm_sctxt::set_storage( cgllvm_sctxt const* rhs ){
	EFLIB_ASSERT_UNIMPLEMENTED();
}

void cgllvm_sctxt::set_type( cgllvm_sctxt const* rhs ){
	EFLIB_ASSERT_UNIMPLEMENTED();
}

void cgllvm_sctxt::set_storage_and_type( cgllvm_sctxt* rhs ){
	set_storage(rhs);
	set_type(rhs);
}

END_NS_SASL_CODE_GENERATOR();