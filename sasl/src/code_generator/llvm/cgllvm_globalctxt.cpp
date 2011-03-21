#include <sasl/include/code_generator/llvm/cgllvm_globalctxt.h>

#include <sasl/include/semantic/abi_analyser.h>
#include <sasl/include/semantic/semantic_infos.h>

#include <eflib/include/platform/disable_warnings.h>
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/Support/IRBuilder.h>
#include <eflib/include/platform/enable_warnings.h>

using sasl::semantic::buffer_in;
using sasl::semantic::buffer_out;
using sasl::semantic::stream_in;
using sasl::semantic::stream_out;

BEGIN_NS_SASL_CODE_GENERATOR();

cgllvm_global_context::cgllvm_global_context()
: mod(NULL), have_mod(true)
{
	lctxt.reset( new llvm::LLVMContext() );
	irbuilder.reset( new llvm::IRBuilder<>( *lctxt ) );
}

void cgllvm_global_context::create_module( const std::string& modname ){
	mod = new llvm::Module( modname, *lctxt );
	have_mod = true;
}

llvm::Module* cgllvm_global_context::module() const{
	return mod;
}

llvm::LLVMContext& cgllvm_global_context::context(){
	return *lctxt;
}

cgllvm_global_context::~cgllvm_global_context(){
	if( have_mod && mod ){
		delete mod;
		mod = NULL;
		have_mod = false;
	}
}

llvm::Module* cgllvm_global_context::get_ownership() const{
	if ( have_mod ){
		have_mod = false;
		return mod;
	}
	return NULL;
}

boost::shared_ptr<llvm::IRBuilder<> > cgllvm_global_context::builder() const{
	return irbuilder;
}

llvm::Type const* cgllvm_global_context::get_type( sasl::semantic::storage_types storage ){

	switch( storage ){

	case stream_in:
		return str_in_struct;
	case stream_out:
		return str_out_struct;
	case buffer_in:
		return buf_in_struct;
	case buffer_out:
		return buf_out_struct;
	}

	return NULL;
}

void cgllvm_global_context::set_type( sasl::semantic::storage_types storage, llvm::Type const* ptype ){
	switch( storage ){

	case stream_in:
		str_in_struct = ptype;
		return;
	case stream_out:
		str_out_struct = ptype;
		return;
	case buffer_in:
		buf_in_struct = ptype;
		return;
	case buffer_out:
		buf_out_struct = ptype;
		return;
	}

	return;
}

END_NS_SASL_CODE_GENERATOR();