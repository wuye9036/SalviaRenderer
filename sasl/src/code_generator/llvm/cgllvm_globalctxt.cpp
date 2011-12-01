#include <sasl/include/code_generator/llvm/cgllvm_globalctxt.h>

#include <sasl/include/semantic/abi_analyser.h>
#include <sasl/include/semantic/semantic_infos.h>

#include <eflib/include/platform/disable_warnings.h>
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/Support/IRBuilder.h>
#include <eflib/include/platform/enable_warnings.h>

using salviar::sv_usage;
using salviar::su_buffer_in;
using salviar::su_buffer_out;
using salviar::su_stream_in;
using salviar::su_stream_out;
using salviar::storage_usage_count;

BEGIN_NS_SASL_CODE_GENERATOR();

cgllvm_modimpl::cgllvm_modimpl()
: mod(NULL), have_mod(true)
{
	lctxt.reset( new llvm::LLVMContext() );
	irbuilder.reset( new llvm::IRBuilder<>( *lctxt ) );
}

void cgllvm_modimpl::create_module( const std::string& modname ){
	mod = new llvm::Module( modname, *lctxt );
	have_mod = true;
}

llvm::Module* cgllvm_modimpl::module() const{
	return mod;
}

llvm::LLVMContext& cgllvm_modimpl::context(){
	return *lctxt;
}

cgllvm_modimpl::~cgllvm_modimpl(){
	if( have_mod && mod ){
		delete mod;
		mod = NULL;
		have_mod = false;
	}
}

llvm::Module* cgllvm_modimpl::get_ownership() const{
	if ( have_mod ){
		have_mod = false;
		return mod;
	}
	return NULL;
}

boost::shared_ptr<llvm::IRBuilder<> > cgllvm_modimpl::builder() const{
	return irbuilder;
}

END_NS_SASL_CODE_GENERATOR();