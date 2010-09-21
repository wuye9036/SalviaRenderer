#ifndef SASL_CODE_GENERATOR_JIT_API_H
#define SASL_CODE_GENERATOR_JIT_API_H

#include <sasl/include/code_generator/forward.h>
#include <boost/shared_ptr.hpp>
BEGIN_NS_SASL_CODE_GENERATOR();

class codegen_context;

class jit_engine{
private:
	jit_engine( boost::shared_ptr<codegen_context> global_ctxt );
	jit_engine( const jit_engine& );
	jit_engine& operator = (const jit_engine& );
};

END_NS_SASL_CODE_GENERATOR();

#endif