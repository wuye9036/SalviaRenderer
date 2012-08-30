#ifndef SASL_CODEGEN_CG_JIT_H
#define SASL_CODEGEN_CG_JIT_H

#include <sasl/include/codegen/forward.h>
#include <sasl/include/codegen/jit_api.h>

#include <string>
#include <vector>

namespace llvm{
	class Function; 
	class ExecutionEngine;
}

BEGIN_NS_SASL_CODEGEN();

class cgllvm_module;
class cgllvm_sctxt;

class cgllvm_jit_engine : public jit_engine{
public:

	static boost::shared_ptr<cgllvm_jit_engine> create( boost::shared_ptr<cgllvm_module>, std::string& error );

	virtual void* get_function( const std::string& /*func_name*/ );
	virtual void inject_function( void* fn, std::string const& );

	virtual ~cgllvm_jit_engine();
protected:
	cgllvm_jit_engine( boost::shared_ptr<cgllvm_module> );
	void build();
	bool is_valid();
	std::string error();
	
private:
	cgllvm_jit_engine( const cgllvm_jit_engine& );
	cgllvm_jit_engine& operator = (const cgllvm_jit_engine& );

	boost::shared_ptr<cgllvm_module> global_ctxt;
	boost::shared_ptr<llvm::ExecutionEngine> engine;
	std::vector<llvm::Function*> fns;
	std::string err;
};

END_NS_SASL_CODEGEN();

#endif