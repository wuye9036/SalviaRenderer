#include <eflib/include/platform/disable_warnings.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/JIT.h>
#include <llvm/Module.h>
#include <llvm/Support/CommandLine.h>
#include <eflib/include/platform/enable_warnings.h>

#include <sasl/include/code_generator/llvm/cgllvm_jit.h>
#include <sasl/include/code_generator/llvm/cgllvm_globalctxt.h>

#include <eflib/include/platform/cpuinfo.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <eflib/include/platform/boost_end.h>

#include <vector>
#include <string>

#include <assert.h>

using std::vector;
using std::string;
using namespace eflib;

struct llvm_options
{
	llvm_options(){
		// Add Options
		char* options[] = { ""/*, "-promote-elements" */};
		llvm::cl::ParseCommandLineOptions( sizeof(options)/sizeof(char*), options );
	}
};

void initialize_llvm_options()
{
	static llvm_options opt;
};

BEGIN_NS_SASL_CODE_GENERATOR();

boost::shared_ptr<cgllvm_jit_engine> cgllvm_jit_engine::create( boost::shared_ptr<cgllvm_module> ctxt, std::string& error ){
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

	llvm::Function* func = global_ctxt->llvm_module()->getFunction( func_name );
	if (!func){
		return NULL;
	}

	void* native_fn = engine->getPointerToFunction( func );
	if( find( fns.begin(), fns.end(), func ) == fns.end() ){
		fns.push_back(func);
	}

	return native_fn;
}

cgllvm_jit_engine::cgllvm_jit_engine( boost::shared_ptr<cgllvm_module> ctxt )
: jit_engine(), global_ctxt( ctxt )
{
	build();
}

void cgllvm_jit_engine::build(){
	if ( !global_ctxt || !global_ctxt->llvm_module() ){
		engine.reset();
	}
	
	initialize_llvm_options();

	// Add Attrs
	vector<string> attrs;
	if( cpu_features( cpu_sse2 ) ){
		attrs.push_back("+sse");
		attrs.push_back("+sse2");
	}
	
	engine.reset(
		llvm::EngineBuilder( global_ctxt->llvm_module() ).setMAttrs(attrs)
		.setErrorStr(&err)
		.create()
		);
	if ( engine ){
		if( !global_ctxt->take_ownership() ){
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

cgllvm_jit_engine::~cgllvm_jit_engine()
{
	BOOST_FOREACH( llvm::Function* fn, fns ){
		engine->freeMachineCodeForFunction( fn );
	}
}

void cgllvm_jit_engine::inject_function( void* fn, std::string const& name )
{
	llvm::Function* func = global_ctxt->llvm_module()->getFunction( name );
	if ( func ){
		engine->addGlobalMapping( func, fn );
	}
}

END_NS_SASL_CODE_GENERATOR();