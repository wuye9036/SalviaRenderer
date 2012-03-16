#include <sasl/include/code_generator/llvm/cgllvm_globalctxt.h>

#include <sasl/include/semantic/abi_analyser.h>
#include <sasl/include/semantic/semantic_infos.h>

#include <eflib/include/platform/disable_warnings.h>
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/Support/IRBuilder.h>
#include <llvm/Support/raw_os_ostream.h>
#include <eflib/include/platform/enable_warnings.h>

using salviar::sv_usage;
using salviar::su_buffer_in;
using salviar::su_buffer_out;
using salviar::su_stream_in;
using salviar::su_stream_out;
using salviar::storage_usage_count;

BEGIN_NS_SASL_CODE_GENERATOR();

llvm_module_impl::llvm_module_impl()
: mod(NULL), have_mod(true)
{
	lctxt.reset( new llvm::LLVMContext() );
	irbuilder.reset( new llvm::IRBuilder<>( *lctxt ) );
}

void llvm_module_impl::create_module( const std::string& modname ){
	mod = new llvm::Module( modname, *lctxt );
	have_mod = true;
}

llvm::Module* llvm_module_impl::module() const{
	return mod;
}

llvm::LLVMContext& llvm_module_impl::context(){
	return *lctxt;
}

llvm_module_impl::~llvm_module_impl(){
	if( have_mod && mod ){
		delete mod;
		mod = NULL;
		have_mod = false;
	}
}

llvm::Module* llvm_module_impl::get_ownership() const{
	if ( have_mod ){
		have_mod = false;
		return mod;
	}
	return NULL;
}

boost::shared_ptr<llvm::IRBuilder<> > llvm_module_impl::builder() const{
	return irbuilder;
}

void llvm_module_impl::dump() const
{
	module()->dump();
}

void llvm_module_impl::dump( std::ostream& ostr ) const
{
	llvm::raw_os_ostream raw_os(ostr);
	module()->print( raw_os, NULL );
	raw_os.flush();
}

END_NS_SASL_CODE_GENERATOR();