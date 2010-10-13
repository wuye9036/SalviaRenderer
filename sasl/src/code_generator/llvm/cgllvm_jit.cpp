#include <eflib/include/disable_warnings.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/JIT.h>
#include <llvm/Module.h>
#include <llvm/Target/TargetSelect.h>
#include <eflib/include/enable_warnings.h>
#include <sasl/include/code_generator/llvm/cgllvm_jit.h>
#include <sasl/include/code_generator/llvm/cgllvm_globalctxt.h>
#include <boost/algorithm/string.hpp>
#include <assert.h>

BEGIN_NS_SASL_CODE_GENERATOR();

boost::shared_ptr<cgllvm_jit_engine> cgllvm_jit_engine::create( boost::shared_ptr<cgllvm_global_context> ctxt, std::string& error ){
	boost::shared_ptr<cgllvm_jit_engine> ret = boost::shared_ptr<cgllvm_jit_engine>( new cgllvm_jit_engine( ctxt ) );
	if( !ret ){
		error.assign( "Unknown error occurred." );
	} else if ( !ret->is_valid() ){
		error = ret->error();
		ret.reset();
	}
	return ret;
}

void* cgllvm_jit_engine::get_function( const std::string& func_name ){
	assert( global_ctxt );
	assert( engine );

	llvm::Function* func = global_ctxt->module()->getFunction( func_name );
	if (!func){
		return NULL;
	}
	return engine->getPointerToFunction( func );
}

cgllvm_jit_engine::cgllvm_jit_engine( boost::shared_ptr<cgllvm_global_context> ctxt )
: jit_engine(), global_ctxt( ctxt )
{
	build();
}

void cgllvm_jit_engine::build(){
	llvm::InitializeNativeTarget();

	if ( !global_ctxt || !global_ctxt->module() ){
		engine.reset();
	}
	engine.reset( llvm::EngineBuilder( global_ctxt->module() ).setErrorStr(&err).create() );
	if ( engine ){
		if( !global_ctxt->get_ownership() ){
			engine.reset();
		}
	}
}

bool cgllvm_jit_engine::is_valid(){
	return (bool)engine;
}

std::string cgllvm_jit_engine::error(){
	return err;
}

END_NS_SASL_CODE_GENERATOR();