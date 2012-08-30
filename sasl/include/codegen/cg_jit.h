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

class cg_module;

class cg_jit_engine : public jit_engine{
public:

	static boost::shared_ptr<cg_jit_engine> create( boost::shared_ptr<cg_module>, std::string& error );

	virtual void* get_function( const std::string& /*func_name*/ );
	virtual void inject_function( void* fn, std::string const& );

	virtual ~cg_jit_engine();
protected:
	cg_jit_engine( boost::shared_ptr<cg_module> );
	void build();
	bool is_valid();
	std::string error();
	
private:
	cg_jit_engine( const cg_jit_engine& );
	cg_jit_engine& operator = (const cg_jit_engine& );

	boost::shared_ptr<cg_module> global_ctxt;
	boost::shared_ptr<llvm::ExecutionEngine> engine;
	std::vector<llvm::Function*> fns;
	std::string err;
};

END_NS_SASL_CODEGEN();

#endif