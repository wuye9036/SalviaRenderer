#ifndef SASL_CODE_GENERATOR_LLVM_UTILITY_H
#define SASL_CODE_GENERATOR_LLVM_UTILITY_H

#include <sasl/include/code_generator/forward.h>

#ifdef FUNCTION_SCOPE
#	error "Redefined FUNCTION_SCOPE."
#endif

#define FUNCTION_SCOPE( fn ) \
	service()->push_fn( (fn) );	\
	scope_guard<void> pop_fn_on_exit##__LINE__( bind( &cg_service::pop_fn, service() ) );


#endif